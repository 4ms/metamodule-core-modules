#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/FM_info.hh"

#include "processors/twoOpFMOscillatorPoly.h"
#include "util/math.hh"

#include <algorithm>
#include <array>
#include <memory>

namespace MetaModule
{

// Polyphonic two-operator FM oscillator.
// The number of voices follows the poly channel count of the V/Oct Carrier
// input (one voice if unpatched or mono). The other CV inputs (V/Oct
// Modulator, Index, Shape, Mix) map onto voices per-channel, with their
// highest channel feeding all the upper voices.
//
// The DSP runs all 4 voices through fixed-size aligned loops so the compiler
// auto-vectorizes them with NEON (see twoOpFMOscillatorPoly.h).
class FMCore : public CoreProcessorPoly {
	using Info = FMInfo;
	using ThisCore = FMCore;

	static constexpr unsigned MaxVoices = MaxPolyChannels;
	static_assert(TwoOpFMPoly::MaxVoices >= MaxVoices);

	static constexpr float CvScale = 1.0f / CvRangeVolts;

public:
	FMCore() {
		set_samplerate(48000);
	}

	void update(void) override {
		const unsigned num_voices = std::clamp<unsigned>(carrierChans, 1, MaxVoices);
		outChans = num_voices;

		if (bypassed) {
			audioOut = {};
			return;
		}

		// Pitch multiples use a table lookup, so only recompute them when a CV changes
		for (unsigned v = 0; v < MaxVoices; v++) {
			float cv = carrierIn[carrierChans ? std::min<unsigned>(v, carrierChans - 1) : 0] * CvScale;
			if (cv != lastCarrierCV[v]) {
				lastCarrierCV[v] = cv;
				carrierMult[v] = MathTools::setPitchMultiple(cv);
			}
		}

		if (modChans) {
			for (unsigned v = 0; v < MaxVoices; v++) {
				float cv = modIn[std::min<unsigned>(v, modChans - 1)] * CvScale;
				if (cv != lastModCV[v]) {
					lastModCV[v] = cv;
					modMult[v] = MathTools::setPitchMultiple(cv);
				}
			}
		}

		// Per-voice controls (fixed-size loops: auto-vectorizable)
		for (unsigned v = 0; v < MaxVoices; v++)
			fm.carrierFreq[v] = basePitch * carrierMult[v];

		if (modChans) {
			// Mod 1V/oct is patched:
			for (unsigned v = 0; v < MaxVoices; v++)
				fm.modFreq[v] = basePitch * modMult[v];
		} else {
			// Mod 1V/oct not patched:
			for (unsigned v = 0; v < MaxVoices; v++)
				fm.modFreq[v] = fm.carrierFreq[v] * ratioFine * ratioCoarse;
		}

		// Index/Shape/Mix are constant while their CV jack is unpatched, so when
		// unpatched they are recomputed only after a knob or patch change dirties
		// them. A patched jack may carry audio-rate CV, so it is recomputed every
		// sample (expand maps the highest channel up to all voices).
		if (indexChans) {
			alignas(16) std::array<float, MaxVoices> indexCV;
			expand(indexIn, indexChans, indexCV);
			for (unsigned v = 0; v < MaxVoices; v++)
				fm.modAmount[v] = MathTools::constrain(indexCV[v] * indexAmount + indexKnob, 0.0f, 1.0f);
		} else if (controlsDirty) {
			fm.modAmount.fill(MathTools::constrain(indexKnob, 0.0f, 1.0f));
		}

		if (shapeChans) {
			alignas(16) std::array<float, MaxVoices> shapeCV;
			expand(shapeIn, shapeChans, shapeCV);
			for (unsigned v = 0; v < MaxVoices; v++)
				fm.shape[v] = MathTools::constrain(shapeCV[v] * shapeAmount + shapeKnob, 0.0f, 1.0f);
		} else if (controlsDirty) {
			fm.shape.fill(MathTools::constrain(shapeKnob, 0.0f, 1.0f));
		}

		if (mixChans) {
			alignas(16) std::array<float, MaxVoices> mixCV;
			expand(mixIn, mixChans, mixCV);
			for (unsigned v = 0; v < MaxVoices; v++)
				fm.mix[v] = MathTools::constrain(mixCV[v] + mix, 0.0f, 1.0f);
		} else if (controlsDirty) {
			fm.mix.fill(MathTools::constrain(mix, 0.0f, 1.0f));
		}

		controlsDirty = false;

		fm.update(num_voices);

		const float *__restrict fmOut = std::assume_aligned<16>(fm.out.data());
		float *__restrict audioOut_ = std::assume_aligned<16>(audioOut.data());
		MM_NO_LOOP_DEPS
		for (unsigned v = 0; v < MaxVoices; v++)
			audioOut_[v] = fmOut[v] * MaxOutputVolts;
	}

	void set_param(int param_id, float val) override {
		controlsDirty = true; // knob change may alter an unpatched control
		switch (param_id) {
			case Info::KnobPitch:
				basePitch = 20.0f * exp5Table.interp(val);
				break;
			case Info::KnobIndex:
				indexKnob = val;
				break;
			case Info::KnobRatio_Coarse:
				ratioCoarse = ratioTable[MathTools::map_value(val, 0.0f, 1.0f, 0, 7)];
				break;
			case Info::KnobRatio_Fine:
				if (val < 0.5f)
					ratioFine = MathTools::map_value(val, 0.0f, 0.5f, 0.5f, 1.0f);
				else
					ratioFine = MathTools::map_value(val, 0.5f, 1.0f, 1.0f, 2.0f);
				break;
			case Info::KnobShape:
				shapeKnob = val;
				break;
			case Info::KnobShape_Cv:
				shapeAmount = val;
				break;
			case Info::KnobIndex_Cv:
				indexAmount = val;
				break;
			case Info::KnobMix:
				mix = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobPitch: {
				auto t = std::lower_bound(exp5Table.data.begin(), exp5Table.data.end(), basePitch / 20.f);
				return std::distance(exp5Table.data.begin(), t) / static_cast<float>(exp5Table.data.size() - 1);
			}
			case Info::KnobIndex:
				return indexKnob;
			case Info::KnobRatio_Coarse: {
				auto t = std::find(ratioTable.begin(), ratioTable.end(), ratioCoarse);
				return std::distance(ratioTable.begin(), t) / static_cast<float>(ratioTable.size() - 1);
			}
			case Info::KnobRatio_Fine:
				if (ratioFine > 1.f)
					return MathTools::map_value(ratioFine, 1.f, 2.f, .5f, 1.f);
				else
					return MathTools::map_value(ratioFine, .5f, 1.f, 0.f, .5f);
			case Info::KnobShape:
				return shapeKnob;
			case Info::KnobShape_Cv:
				return shapeAmount;
			case Info::KnobIndex_Cv:
				return indexAmount;
			case Info::KnobMix:
				return mix;
		}
		return 0;
	}

	// Mono sources write channel 0; poly sources write the buffers directly
	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputIndex_Cv_In:
				indexIn[0] = val;
				break;
			case Info::InputV_Oct_Carrier:
				carrierIn[0] = val;
				break;
			case Info::InputShape_Cv_In:
				shapeIn[0] = val;
				break;
			case Info::InputMix_Cv_In:
				mixIn[0] = val;
				break;
			case Info::InputV_Oct_Modulator:
				modIn[0] = val;
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
			case Info::InputV_Oct_Carrier:
				return {carrierIn.data(), &carrierChans};
			case Info::InputV_Oct_Modulator:
				return {modIn.data(), &modChans};
			case Info::InputMix_Cv_In:
				return {mixIn.data(), &mixChans};
			case Info::InputIndex_Cv_In:
				return {indexIn.data(), &indexChans};
			case Info::InputShape_Cv_In:
				return {shapeIn.data(), &shapeChans};
		}
		return {};
	}

	PolyPortBuffer get_poly_output_buffer(int output_id) override {
		if (output_id == Info::OutputAudio_Out)
			return {audioOut.data(), &outChans};
		return {};
	}

	void mark_all_inputs_unpatched() override {
		for (int input_id = 0; input_id < Info::NumInJacks; input_id++)
			mark_input_unpatched(input_id);
	}

	void mark_input_unpatched(int input_id) override {
		if (auto *jack = jack_for(input_id)) {
			*jack->chans = 0;
			*jack->values = {};
			controlsDirty = true; // patch change may alter an unpatched control
		}
	}

	void mark_input_patched(int input_id) override {
		// 0 -> 1 channel; a poly source will then raise the count itself
		if (auto *jack = jack_for(input_id)) {
			if (*jack->chans == 0) {
				*jack->chans = 1;
				*jack->values = {};
				controlsDirty = true; // patch change may alter an unpatched control
			}
		}
	}

	void set_samplerate(float sr) override {
		fm.set_samplerate(sr);
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
	static void expand(std::array<float, MaxVoices> const &in, uint8_t chans, std::array<float, MaxVoices> &out) {
		if (chans == 0) {
			out = {};
		} else {
			for (unsigned v = 0; v < MaxVoices; v++)
				out[v] = in[std::min<unsigned>(v, chans - 1)] * CvScale;
		}
	}

	struct JackRef {
		std::array<float, MaxVoices> *values;
		uint8_t *chans;
	};

	JackRef *jack_for(int input_id) {
		static_assert(Info::NumInJacks == 5);
		switch (input_id) {
			case Info::InputV_Oct_Carrier:
				return &jacks[0];
			case Info::InputV_Oct_Modulator:
				return &jacks[1];
			case Info::InputMix_Cv_In:
				return &jacks[2];
			case Info::InputIndex_Cv_In:
				return &jacks[3];
			case Info::InputShape_Cv_In:
				return &jacks[4];
		}
		return nullptr;
	}

	// Poly port storage:
	alignas(16) std::array<float, MaxVoices> carrierIn{};
	alignas(16) std::array<float, MaxVoices> modIn{};
	alignas(16) std::array<float, MaxVoices> mixIn{};
	alignas(16) std::array<float, MaxVoices> indexIn{};
	alignas(16) std::array<float, MaxVoices> shapeIn{};
	alignas(16) std::array<float, MaxVoices> audioOut{};
	uint8_t carrierChans = 0;
	uint8_t modChans = 0;
	uint8_t mixChans = 0;
	uint8_t indexChans = 0;
	uint8_t shapeChans = 0;
	uint8_t outChans = 1;

	std::array<JackRef, 5> jacks{{
		{&carrierIn, &carrierChans},
		{&modIn, &modChans},
		{&mixIn, &mixChans},
		{&indexIn, &indexChans},
		{&shapeIn, &shapeChans},
	}};

	// Per-voice pitch caches (recomputed only when a CV changes):
	alignas(16) std::array<float, MaxVoices> carrierMult{1.f, 1.f, 1.f, 1.f};
	alignas(16) std::array<float, MaxVoices> modMult{1.f, 1.f, 1.f, 1.f};
	std::array<float, MaxVoices> lastCarrierCV{};
	std::array<float, MaxVoices> lastModCV{};

	TwoOpFMPoly fm;

	float ratioCoarse = 1;
	float ratioFine = 1;
	float mix = 0;
	float basePitch = 0;
	float indexKnob = 0;
	float shapeKnob = 0;
	float shapeAmount = 0;
	float indexAmount = 0;

	// Set when a knob or patch change may have altered an unpatched Index/Shape/
	// Mix control, so update() recomputes those once instead of every sample.
	bool controlsDirty = true;

	const std::array<float, 8> ratioTable = {0.125f, 0.25f, 0.5f, 1, 2, 4, 8, 16};
};

} // namespace MetaModule
