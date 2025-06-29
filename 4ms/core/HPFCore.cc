#include "CoreModules/moduleFactory.hh"
#include "info/HPF_info.hh"
#include "processors/hpf.h"
#include "processors/korgHPF.h"
#include "util/math.hh"

namespace MetaModule
{

class HPFCore : public CoreProcessor {
	using Info = HPFInfo;
	using ThisCore = HPFCore;

public:
	HPFCore() = default;

	void update() override {
		float filterFreq = setPitchMultiple(constrain(cutoffOffset + cutoffCV, -1.0f, 1.0f)) * 523.25f;
		if (mode == 0) {
			hpf.cutoff.setValue(filterFreq);
			signalOutput = hpf.update(signalInput);
		} else if (mode == 1) {
			korg.cutoff.setValue(audioFreqToNorm(filterFreq));
			signalOutput = korg.update(signalInput);
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobCutoff:
				cutoffOffset = map_value(val, 0.0f, 1.0f, -1.0f, 1.0f);
				break;
			case Info::KnobQ:
				if (mode == 0) {
					hpf.q = map_value(val, 0.0f, 1.0f, 1.0f, 20.0f);
				} else if (mode == 1) {
					korg.q = map_value(val, 0.0f, 1.0f, 0.0f, 10.0f);
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
					return map_value(hpf.q, 1.f, 20.f, 0.f, 1.f);
				else if (mode == 1)
					return map_value(korg.q, 0.f, 10.f, 0.f, 1.f);
				break;
			case (Info::SwitchMode + 2): //Info::NumKnobs
				return mode == 1 ? 1.f : 0.f;
		}
		return 0;
	}

	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputAudio_In:
				signalInput = val;
				break;
			case Info::InputCutoff_Cv_In:
				cutoffCV = val / CvRangeVolts;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return signalOutput;
		return 0.f;
	}

	void set_samplerate(float sr) override {
		hpf.sampleRate.setValue(sr);
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
	HighPassFilter hpf;
	KorgHPF korg;
	float signalInput = 0;
	float signalOutput = 0;
	float cutoffOffset = 0;
	float cutoffCV = 0;
};

} // namespace MetaModule
