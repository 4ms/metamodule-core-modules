#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/HPF_info.hh"
#include "processors/hpf.h"
#include "processors/korgHPF.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// filter state. The Cutoff CV's highest channel feeds all upper voices.
class HPFCore : public PolyCoreProcessor<HPFInfo::NumInJacks, HPFInfo::NumOutJacks> {
	using Info = HPFInfo;
	using ThisCore = HPFCore;

public:
	HPFCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &out = outs[Info::OutputAudio_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		auto &cutoff = ins[Info::InputCutoff_Cv_In];

		for (unsigned v = 0; v < nv; v++) {
			float cutoffCV = cutoff.or_last(v) / CvRangeVolts;
			float filterFreq = setPitchMultiple(constrain(cutoffOffset + cutoffCV, -1.0f, 1.0f)) * 523.25f;
			if (mode == 0) {
				hpf[v].cutoff.setValue(filterFreq);
				out.values[v] = hpf[v].update(in.values[v]);
			} else if (mode == 1) {
				korg[v].cutoff.setValue(audioFreqToNorm(filterFreq));
				out.values[v] = korg[v].update(in.values[v]);
			}
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobCutoff:
				cutoffOffset = map_value(val, 0.0f, 1.0f, -1.0f, 1.0f);
				break;
			case Info::KnobQ:
				if (mode == 0) {
					for (auto &f : hpf)
						f.q = map_value(val, 0.0f, 1.0f, 1.0f, 20.0f);
				} else if (mode == 1) {
					for (auto &f : korg)
						f.q = map_value(val, 0.0f, 1.0f, 0.0f, 10.0f);
				}
				break;
			case (Info::SwitchMode + 2): //Info::NumKnobs
				mode = val > .5f ? 1 : 0;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobCutoff:
				return map_value(cutoffOffset, -1.f, 1.f, 0.f, 1.f);
			case Info::KnobQ:
				if (mode == 0)
					return map_value(hpf[0].q, 1.f, 20.f, 0.f, 1.f);
				else if (mode == 1)
					return map_value(korg[0].q, 0.f, 10.f, 0.f, 1.f);
				break;
			case (Info::SwitchMode + 2): //Info::NumKnobs
				return mode == 1 ? 1.f : 0.f;
		}
		return 0;
	}

	void set_samplerate(float sr) override {
		for (auto &f : hpf)
			f.sampleRate.setValue(sr);
	}

	float get_led_brightness(int led_id) const override {
		return mode;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	int mode = 0;
	std::array<HighPassFilter, MaxVoices> hpf{};
	std::array<KorgHPF, MaxVoices> korg{};
	float cutoffOffset = 0;
};

} // namespace MetaModule
