#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Pan_info.hh"

#include "util/math.hh"

namespace MetaModule
{

class PanCore : public CoreProcessor {
	using Info = PanInfo;
	using ThisCore = PanCore;

public:
	PanCore() = default;

	void update() override {
		if (bypassed) {
			leftOut = signalInput;
			rightOut = signalInput;
			return;
		}

		float finalPan = MathTools::constrain(panPosition + panCV, 0.0f, 1.0f);
		leftOut = signalInput * (1.0f - finalPan);
		rightOut = signalInput * finalPan;
	}

	void set_param(int param_id, float val) override {
		if (param_id == Info::KnobPan)
			panPosition = val;
	}

	float get_param(int param_id) const override {
		if (param_id == Info::KnobPan)
			return panPosition;
		return 0;
	}

	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputAudio_In:
				signalInput = val;
				break;
			case Info::InputPan_Cv_In:
				panCV = val / CvRangeVolts;
		}
	}

	float get_output(int output_id) const override {
		switch (output_id) {
			case Info::OutputCh__1_Out:
				return leftOut;

			case Info::OutputCh__2_Out:
				return rightOut;
		}
		return 0.f;
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
	float panPosition = 0;
	float signalInput = 0;
	float leftOut = 0;
	float rightOut = 0;
	float panCV = 0;
};

} // namespace MetaModule
