#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "helpers/EdgeDetector.h"
#include "helpers/FlipFlop.h"
#include "info/SH_info.hh"

namespace MetaModule
{

class SHCore : public CoreProcessor {
	using Info = SHInfo;
	using ThisCore = SHCore;

public:
	SHCore()
		: triggerDetector{{0.1f, 0.2f}, {0.1f, 0.2f}} {
	}

	void update() override {
	}

	void set_param(int param_id, float val) override {
	}

	float get_param(int param_id) const override {
		return 0;
	}

	void set_input(int input_id, float val) override {
		if (bypassed)
			return;

		switch (input_id) {
			case Info::InputCh__1_Clock_In:
				input[0] = val;
				break;

			case Info::InputCh__1_Sample_In:
				if (trig[0](triggerDetector[0](val))) {
					held[0] = input[0];
				}
				break;

			case Info::InputCh__2_Clock_In:
				input[1] = val;
				break;

			case Info::InputCh__2_Sample_In:
				if (trig[1](triggerDetector[1](val))) {
					held[1] = input[1];
				}
				break;
		}
	}

	float get_output(int output_id) const override {
		if (bypassed)
			return 0;

		if (output_id < 0 || output_id > 1)
			return 0;

		return held[output_id];
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
	float held[2]{};
	float input[2]{};

	FlipFlop triggerDetector[2];
	EdgeDetector trig[2];
};

} // namespace MetaModule
