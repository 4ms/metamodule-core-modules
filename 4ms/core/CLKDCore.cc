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

	std::array<tvg::SwCanvas *, 1> canvas = {tvg::SwCanvas::gen()};
	std::array<float, 1> scaling = {1};

	~CLKDCore() = default;

	void show_graphic_display(int display_id, Pixel *pix, uint16_t width, uint16_t height) override {
		canvas[0]->target(reinterpret_cast<uint32_t *>(pix), width, width, height, tvg::ColorSpace::ARGB8888);
		scaling[0] = float(width) / base_element(Info::Elements[4]).width_mm;
	}

	void hide_graphic_display(int display_id) override {
		// scene->remove();
		canvas[0]->remove();
	}

	bool get_canvas_pixels(int display_id) override {
		tvg::Scene *scene = tvg::Scene::gen();
		// scene->remove();

		for (auto i = 0u; i < 2; i++) {
			for (auto j = 0u; j < 4; j++) {
				auto sq = tvg::Shape::gen();
				sq->appendRect(i * 10, j * 10, 7, 7);
				sq->fill(i * 0x60, j * 0x30, 0xFF, 0xFF);
				scene->push(sq);
			}
		}

		auto circle = tvg::Shape::gen();
		circle->appendCircle(15, 25, 15, 25);
		circle->fill(0x00, 0x80, 0xFF, 0xCC);
		scene->push(circle);

		auto circle2 = tvg::Shape::gen();
		circle2->appendCircle(5, 45, 10, clockDivideOffset * 25);
		circle2->fill(0xFF, 0x00, 0x00, 0xFF);
		scene->push(circle2);

		scene->scale(scaling[0]);

		canvas[0]->remove();
		canvas[0]->push(scene);
		canvas[0]->draw();
		canvas[0]->sync();

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
