#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/CLKD_info.hh"
#include "processors/tools/clockPhase.h"
#include "util/math.hh"
#include <cmath>

using namespace MathTools;

namespace MetaModule
{

class CLKDCore : public CoreProcessor {
	using Info = CLKDInfo;
	using ThisCore = CLKDCore;

public:
	CLKDCore() = default;

	void update() override {
		cp.update();
		if ((cp.getWrappedPhase() < pulseWidth) && clockInit) {
			clockOutput = gateVoltage;
		} else {
			clockOutput = 0;
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobDivide:
				clockDivideOffset = val;
				update_divider();
				break;
		}
	}

	float get_param(int param_id) const override {
		if (param_id == Info::KnobDivide)
			return std::round(clockDivideOffset * (maxDivider - 1)) / (maxDivider - 1);
		return 0;
	}

	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputClk_In:
				cp.updateClock(val);
				clockInit = true;
				break;
			case Info::InputCv: {
				float tmp = val / CvRangeVolts;
				if (tmp != clockDivideCV) {
					clockDivideCV = tmp;
					update_divider();
				}
			} break;
		}
	}

	float get_output(int output_id) const override {
		return clockOutput;
	}

	void set_samplerate(float sr) override {
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	void update_divider() {
		float finalDivide = std::clamp(clockDivideOffset + clockDivideCV, 0.0f, 1.0f);
		cp.setDivide(std::round(map_value(finalDivide, 0.0f, 1.0f, 1.0f, maxDivider)));
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	static constexpr float pulseWidth = 0.5f;
	int clockOutput = 0;
	bool clockInit = false;
	float clockDivideOffset = 0;
	float clockDivideCV = 0;

	ClockPhase cp;

	static constexpr float gateVoltage = 8.0f;
	static constexpr float maxDivider = 16.f;
};

} // namespace MetaModule
