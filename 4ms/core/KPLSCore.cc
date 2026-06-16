#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/KPLS_info.hh"

#include "processors/envelope.h"
#include "processors/karplus.h"
#include "util/math.hh"

#include <algorithm>
#include <array>

namespace MetaModule
{

// Polyphonic Karplus-Strong voice.
// The number of voices follows the poly channel count of the V/Oct input
// (one voice if unpatched or mono). If the Trigger input has fewer channels
// than V/Oct, its highest channel triggers all the upper voices.
class KPLSCore : public CoreProcessorPoly {
	using Info = KPLSInfo;
	using ThisCore = KPLSCore;

	static constexpr unsigned MaxVoices = MaxPolyChannels;
	static_assert(Karplus::MaxVoices >= MaxVoices);

public:
	KPLSCore() {
		for (auto &e : envs) {
			e.set_sustain(0.0f);
			e.set_envelope_time(0, 0.1);
			e.set_envelope_time(1, 0);
			e.set_envelope_time(2, 30);
			e.set_decay_curve(0);
			e.set_release_curve(0.5f);
			e.set_sustain(0.1f);
			e.set_envelope_time(3, 200);
			e.sustainEnable = 0;
		}
	}

	void update() override {
		const unsigned num_voices = std::clamp<unsigned>(voctChans, 1, MaxVoices);
		outChans = num_voices;

		if (bypassed) {
			audioOut = {};
			return;
		}

		alignas(16) std::array<float, MaxVoices> burst{};
		for (unsigned v = 0; v < MaxVoices; v++) {
			// Run every envelope, even for inactive voices, so an inactive voice's
			// envelope is not stuck mid-cycle when the voice becomes active
			float gate = trigChans ? trigIn[std::min<unsigned>(v, trigChans - 1)] : 0.f;
			float env = envs[v].update(gate / CvRangeVolts);

			if (v < num_voices) {
				// The pitch->multiple table lookup depends only on the raw V/Oct
				// volts, so recompute it only when that input actually changes (it
				// otherwise runs every sample). basePitch (knob) is folded in
				// cheaply afterwards, so a knob move is still picked up at once.
				if (voctIn[v] != lastVoctIn[v]) {
					lastVoctIn[v] = voctIn[v];
					auto pitchCV = MathTools::constrain(voctIn[v] / CvRangeVolts, 0.0f, 1.0f);
					pitchMult[v] = exp5Table.interp(pitchCV);
				}
				auto freq = basePitch * pitchMult[v];
				if (freq != voiceFreq[v]) {
					voiceFreq[v] = freq;
					k.set_frequency(v, freq);
				}

				burst[v] = noise(v) * env;
			}
		}

		alignas(16) std::array<float, MaxVoices> karpOut{};
		k.update(burst.data(), karpOut.data(), num_voices);

		for (unsigned v = 0; v < num_voices; v++)
			audioOut[v] = karpOut[v] * MaxOutputVolts;
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobPitch:
				basePitch = MathTools::map_value(val, 0.0f, 1.0f, 20.0f, 400.0f);
				break;
			case Info::KnobDecay:
				k.set_decay(val);
				for (auto &e : envs) {
					e.set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.2f));
					e.set_envelope_time(3, MathTools::map_value(val, 0.0f, 1.0f, 200.0f, 1000.0f));
				}
				break;
			case Info::KnobSpread:
				k.set_spread(val);
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobPitch:
				return MathTools::map_value(basePitch, 20.f, 400.f, 0.f, 1.f);
			case Info::KnobDecay:
				return k.get_decay();
			case Info::KnobSpread:
				return k.get_spread();
		}
		return 0;
	}

	// Mono sources write channel 0; poly sources write the buffers directly
	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputTrigger_In:
				trigIn[0] = val;
				break;
			case Info::InputV_Oct_In:
				voctIn[0] = val;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return audioOut[0];
		return 0.f;
	}

	PolyPortBuffer get_poly_input_buffer(int input_id) override {
		switch (input_id) {
			case Info::InputTrigger_In:
				return {trigIn.data(), &trigChans};
			case Info::InputV_Oct_In:
				return {voctIn.data(), &voctChans};
		}
		return {};
	}

	PolyPortBuffer get_poly_output_buffer(int output_id) override {
		if (output_id == Info::OutputAudio_Out)
			return {audioOut.data(), &outChans};
		return {};
	}

	void mark_all_inputs_unpatched() override {
		mark_input_unpatched(Info::InputTrigger_In);
		mark_input_unpatched(Info::InputV_Oct_In);
	}

	void mark_input_unpatched(int input_id) override {
		switch (input_id) {
			case Info::InputTrigger_In:
				trigChans = 0;
				trigIn = {};
				break;
			case Info::InputV_Oct_In:
				voctChans = 0;
				voctIn = {};
				break;
		}
	}

	void mark_input_patched(int input_id) override {
		// 0 -> 1 channel; a poly source will then raise the count itself
		switch (input_id) {
			case Info::InputTrigger_In:
				if (trigChans == 0) {
					trigChans = 1;
					trigIn = {};
				}
				break;
			case Info::InputV_Oct_In:
				if (voctChans == 0) {
					voctChans = 1;
					voctIn = {};
				}
				break;
		}
	}

	void set_samplerate(float sr) override {
		k.set_samplerate(sr);
		for (auto &e : envs)
			e.set_samplerate(sr);
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	float noise(unsigned v) {
		noiseState[v] = noiseState[v] * 1103515245u + 12345u;
		return float(int32_t(noiseState[v])) * (1.f / 2147483648.f);
	}

	alignas(16) std::array<float, MaxVoices> voctIn{};
	alignas(16) std::array<float, MaxVoices> trigIn{};
	alignas(16) std::array<float, MaxVoices> audioOut{};
	uint8_t voctChans = 0;
	uint8_t trigChans = 0;
	uint8_t outChans = 1;

	float basePitch = 20;
	std::array<float, MaxVoices> voiceFreq{};
	// Cache for the per-voice pitch lookup (recomputed only when V/Oct changes).
	// lastVoctIn is seeded with a sentinel no real input can equal, so the first
	// update() always computes pitchMult.
	std::array<float, MaxVoices> lastVoctIn{-1e9f, -1e9f, -1e9f, -1e9f};
	std::array<float, MaxVoices> pitchMult{};
	std::array<uint32_t, MaxVoices> noiseState{0x12345678u, 0x9abcdef0u, 0xfedcba98u, 0x76543210u};

	Karplus k;
	std::array<Envelope, MaxVoices> envs;
};

} // namespace MetaModule
