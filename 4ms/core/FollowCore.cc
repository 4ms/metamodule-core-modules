#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Follow_info.hh"
#include "processors/tools/expDecay.h"
#include "processors/tools/schmittTrigger.h"
#include "util/math.hh"

namespace MetaModule
{

class FollowCore : public CoreProcessor {
	using Info = FollowInfo;
	using ThisCore = FollowCore;

public:
	FollowCore() = default;

	void update(void) override {
		if (bypassed) {
			envOutput = 0;
			gateOutput = 0;
			return;
		}

		float rectSignal = signalInput;
		if (rectSignal < 0)
			rectSignal *= -1.0f;
		envOutput = slew.update(rectSignal);
		wc.update(envOutput);
		gateOutput = wc.output();
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobThreshold: //threshold
			{
				float topThresh;
				float bottomThresh;
				constexpr float errorAmount = 0.1f;
				topThresh = val + errorAmount;
				if (topThresh > 1.0f)
					topThresh = 1.0f;
				bottomThresh = val - errorAmount;
				if (bottomThresh < 0)
					bottomThresh = 0;
				wc.setHighThreshold(topThresh);
				wc.setLowThreshhold(bottomThresh);
			} break;
			case Info::KnobRise: //rise
				slew.set_attack_ms(MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 2000.f));
				break;
			case Info::KnobFall: //fall
				slew.set_decay_ms(MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 2000.0f));
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobThreshold: //threshold
				return wc.getLowThreshold();
			case Info::KnobRise: //rise
				return MathTools::map_value(slew.attack_ms(), 1.f, 2000.f, 0.f, 1.f);
			case Info::KnobFall: //fall
				return MathTools::map_value(slew.decay_ms(), 1.f, 2000.f, 0.f, 1.f);
		}
		return 0;
	}

	void set_input(int input_id, float val) override {
		if (input_id == Info::InputSignal_In)
			signalInput = val / maxOutputVolts;
	}

	float get_output(int output_id) const override {
		switch (output_id) {
			case Info::OutputEnvelope_Out:
				return envOutput * maxOutputVolts;
			case Info::OutputGate_Out:
				return gateOutput * maxOutputVolts;
			default:
				return 0.f;
		}
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
	float signalInput = 0;
	float envOutput = 0;
	float gateOutput = 0;
	SchmittTrigger wc;
	ExpDecay slew;

	static constexpr float cvRangeVolts = 5.0f;
	static constexpr float maxOutputVolts = 8.0f;
};

} // namespace MetaModule
