#include "../../../../src/medium/debug_raw.h"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/CLKD_info.hh"
#include "processors/tools/clockPhase.h"
#include "thorvg/thorvg/inc/thorvg.h"
#include "util/math.hh"

using namespace MathTools;

namespace MetaModule
{

class CLKDCore : public CoreProcessor {
	using Info = CLKDInfo;
	using ThisCore = CLKDCore;

public:
	CLKDCore() {
	}

	// tvg::SwCanvas canvas;

	std::unique_ptr<tvg::SwCanvas> canvas = tvg::SwCanvas::gen();
	// tvg::Shape *circle = tvg::Shape::gen();
	// tvg::Shape *circle2 = tvg::Shape::gen();

	~CLKDCore() {
	}

	bool get_canvas_pixels(int display_id, Pixel *pix, uint16_t width, uint16_t height) override {
		// auto canvas = MetaModule::make_thorvg_canvas(pix, width, height);

		static bool first = true;
		// if (first) {
		// 	first = false;
		canvas->target(reinterpret_cast<uint32_t *>(pix), width, width, height, tvg::SwCanvas::Colorspace::ARGB8888);
		// }
		// canvas->clear();

		//100us
		auto circle = tvg::Shape::gen();
		circle->appendCircle(15, 25, 15, 25);
		circle->fill(0xFF, 0x80, 0x00, 0xCC);
		canvas->push(std::move(circle));

		//100us
		auto circle2 = tvg::Shape::gen();
		circle2->appendCircle(5, 45, 10, clockDivideOffset * 25);
		circle2->fill(0x00, 0x80, 0xFF, 0x30);
		canvas->push(std::move(circle2));

		// canvas->draw(true);
		// canvas->sync();

		// circle2->moveTo(clockDivideOffset * 20, 45);
		// 65ms
		canvas->draw();

		//<200ns
		canvas->sync();

		// delete circle;
		// delete circle2;
		// delete canvas;

		// for (uint16_t y = 0; y < height; y++) {
		// 	for (uint16_t x = 0; x < width; x++) {
		// 		if (y < height / 2) {
		// 			if (x < width / 2)
		// 				pix[x + y * width] = Pixel{0xFF, 0x00, 0x00, 0xFF};
		// 			else
		// 				pix[x + y * width] = Pixel{0x80, 0xA0, 0xC0, 0x8F};
		// 		} else {
		// 			pix[x + y * width] = Pixel{0x44, (x < width / 2) ? uint8_t(0x99) : uint8_t(0xFF), 0x00, 0x7F};
		// 		}
		// 	}
		// }
		return true;
	}

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
		switch (param_id) {
			case Info::KnobDivide:
				return clockDivideOffset;
		}
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
		cp.setDivide(map_value(finalDivide, 0.0f, 1.0f, 1.0f, 16.99f));
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
};

} // namespace MetaModule
