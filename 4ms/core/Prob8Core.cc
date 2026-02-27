#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Prob8_info.hh"
#include "processors/tools/clockPhase.h"
#include "util/math.hh"

namespace MetaModule
{

class Prob8Core : public CoreProcessor {
	using Info = Prob8Info;
	using ThisCore = Prob8Core;

public:
	Prob8Core() = default;

	void update() override {
		if (bypassed) {
			return;
		}

		cp.update();
		lastStep = currentStep;
		currentStep = cp.getCount() % 8;
		if (currentStep != lastStep) {
			randNum = MathTools::randomNumber(0.0f, 0.99f);
		}
		if ((prob[currentStep] > randNum) && (cp.getWrappedPhase() < 0.5f)) {
			gateOutput = 1;
		} else {
			gateOutput = 0;
		}
	}

	void set_param(int param_id, float val) override {
		if (param_id < Info::NumKnobs)
			prob[param_id] = val;
	}

	float get_param(int param_id) const override {
		if (param_id < Info::NumKnobs)
			return prob[param_id];
		return 0;
	}

	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputClock_In:
				cp.updateClock(val);
				break;
			case Info::InputReset_Gate_In:
				cp.updateReset(val);
				break;
		}
	}

	float get_output(int output_id) const override {
		if (bypassed)
			return 0;

		if (output_id == Info::OutputProbability_Output)
			return gateOutput * MaxOutputVolts;

		if (output_id == Info::OutputInverted_Out)
			return (1 - gateOutput) * MaxOutputVolts;

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
	float prob[8] = {1, 0, 0, 0, 0, 0, 0, 0};

	int gateOutput;

	int currentStep;
	int lastStep;

	float randNum = 0;
	ClockPhase cp;
};

} // namespace MetaModule
