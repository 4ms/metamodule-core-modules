#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Verb_info.hh"
#include "l4/DCBlock.h"
#include "processors/allpass.h"
#include "processors/comb.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice is an
// independent reverb. The CV inputs map per voice, with their highest
// channels feeding all upper voices. Filter tunings use the same
// knob+CV combination as the mono version, recalculated per voice only when
// its effective value changes.
class VerbCore : public PolyCoreProcessor<VerbInfo::NumInJacks, VerbInfo::NumOutJacks> {
	using Info = VerbInfo;
	using ThisCore = VerbCore;

public:
	VerbCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &out = outs[Info::OutputAudio_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		for (unsigned v = 0; v < nv; v++) {
			auto &voice = voices[v];

			update_voice_params(v);

			float wetSignal = 0;

			for (int i = 0; i < numComb; i++) {
				wetSignal += voice.combFilter[i].process(in.values[v]);
			}

			wetSignal /= static_cast<float>(numComb);

			for (int i = 0; i < numAllpass; i++) {
				wetSignal = voice.apFilter[i].process(wetSignal);
			}

			float mix = combine_cv_and_pot(mix_knob, ins[Info::InputMix_Cv].or_last(v));
			out.values[v] =
				std::clamp(voice.dc_blocker(MathTools::interpolate(in.values[v], wetSignal, mix)), -10.f, 10.f);
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case VerbInfo::KnobSize:
				size_knob = val;
				break;

			case VerbInfo::KnobDamping:
				damp_knob = val;
				break;

			case VerbInfo::KnobMix:
				mix_knob = val;
				break;

			case VerbInfo::KnobTime:
				feedback_knob = val;
				break;

			case VerbInfo::KnobAp_Ratio:
				ap_ratio_knob = val;
				break;

			case VerbInfo::KnobComb:
				comb_ratio_knob = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case VerbInfo::KnobSize:
				return size_knob;
			case VerbInfo::KnobDamping:
				return damp_knob;
			case VerbInfo::KnobMix:
				return mix_knob;
			case VerbInfo::KnobTime:
				return feedback_knob;
			case VerbInfo::KnobAp_Ratio:
				return ap_ratio_knob;
			case VerbInfo::KnobComb:
				return comb_ratio_knob;
		}
		return 0;
	}

	void set_samplerate(float sr) override {
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
	static constexpr int numAllpass = 4;
	static constexpr int numComb = 8;

	static constexpr int maxAllpassTuning[numAllpass] = {6000, 6000, 6000, 6000};
	static constexpr int maxCombTuning[numComb] = {6000, 6000, 6000, 6000, 6000, 6000, 6000, 6000};

	struct Voice {
		AllPass<6000> apFilter[numAllpass];
		Comb<6000> combFilter[numComb];
		DCBlock dc_blocker{0.9995f};

		// Last applied effective (knob+CV) values; -1 forces the initial apply
		float applied_size = -1.f;
		float applied_damp = -1.f;
		float applied_fb = -1.f;
		float applied_ap_ratio = -1.f;
		float applied_comb_ratio = -1.f;

		Voice() {
			for (int i = 0; i < numAllpass; i++) {
				apFilter[i].setLength(maxAllpassTuning[i]);
				apFilter[i].setFeedback(0.6f);
				apFilter[i].setFadeSpeed(0.001f);
			}

			for (int i = 0; i < numComb; i++) {
				combFilter[i].setFeedback(0);
				combFilter[i].setLength(maxCombTuning[i]);
				combFilter[i].setFadeSpeed(0.001f);
			}
		}
	};

	static float combine_cv_and_pot(float pot, float cv) {
		return std::clamp(pot + (cv / 5), 0.f, 1.f);
	}

	// Ratio knob steps: 0..48 in steps of 1/12, giving divisors 1..5
	static float ratio_divisor(float r) {
		int ival = (int)(r * 48);
		return ival / 12.f + 1.f;
	}

	void update_voice_params(unsigned v) {
		auto &voice = voices[v];

		const float size = combine_cv_and_pot(size_knob, ins[Info::InputSize_Cv].or_last(v));
		const float ap_ratio = combine_cv_and_pot(ap_ratio_knob, ins[Info::InputRatio_Cv].or_last(v));
		const float comb_ratio = combine_cv_and_pot(comb_ratio_knob, ins[Info::InputComb_Cv].or_last(v));

		if (size != voice.applied_size || ap_ratio != voice.applied_ap_ratio) {
			const float fval = ratio_divisor(ap_ratio);
			float ratioAtten = 1.f;
			for (int i = 0; i < numAllpass; i++) {
				if (i > 0)
					ratioAtten /= fval;
				float tuning = maxAllpassTuning[i] * size * ratioAtten;
				voice.apFilter[i].setLength(std::max(tuning, 1.f));
			}
			voice.applied_ap_ratio = ap_ratio;
		}

		if (size != voice.applied_size || comb_ratio != voice.applied_comb_ratio) {
			const float fval = ratio_divisor(comb_ratio);
			float ratioAtten = 1.f;
			for (int i = 0; i < numComb; i++) {
				if (i > 0)
					ratioAtten /= fval;
				float tuning = maxCombTuning[i] * size * ratioAtten;
				voice.combFilter[i].setLength(std::max(tuning, 1.f));
			}
			voice.applied_comb_ratio = comb_ratio;
		}

		voice.applied_size = size;

		if (const float damp = combine_cv_and_pot(damp_knob, ins[Info::InputDamp_Cv].or_last(v));
			damp != voice.applied_damp) {
			voice.applied_damp = damp;
			for (int i = 0; i < numComb; i++) {
				voice.combFilter[i].setDamp(damp);
			}
		}

		if (const float fb = combine_cv_and_pot(feedback_knob, ins[Info::InputTime_Cv].or_last(v));
			fb != voice.applied_fb) {
			voice.applied_fb = fb;
			const float fbVal = MathTools::map_value(fb, 0.0f, 1.0f, 0.8f, 0.99f);
			for (int i = 0; i < numComb; i++) {
				voice.combFilter[i].setFeedback(fbVal);
			}
		}
	}

	std::array<Voice, MaxVoices> voices{};

	float size_knob = 0.f;
	float damp_knob = 0.f;
	float mix_knob = 0.f;
	float feedback_knob = 0.f;
	float ap_ratio_knob = 0.f;
	float comb_ratio_knob = 0.f;
};

} // namespace MetaModule
