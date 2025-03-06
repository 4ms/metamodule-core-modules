#include "CoreModules/CoreHelper.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/CLKD_info.hh"
#include "processors/tools/clockPhase.h"
#include "thorvg/thorvg/inc/thorvg.h"
#include "util/math.hh"
#include <chrono>

using namespace MathTools;

namespace MetaModule
{

class CLKDCore : public CoreProcessor {
	using Info = CLKDInfo;
	using ThisCore = CLKDCore;
	using CoreHelper = CoreHelper<Info>;
	using enum Info::Elem;

public:
	CLKDCore() = default;

	std::array<tvg::SwCanvas *, 2> canvas;
	std::array<tvg::Scene *, 2> scene;

	tvg::Shape *bg;
	tvg::Shape *circle;
	tvg::Shape *oval;

	tvg::Shape *bg2;
	tvg::Shape *rect2;

	~CLKDCore() = default;

	void show_graphic_display(int display_id, std::span<uint32_t> pix, uint16_t width, uint16_t height) override {
		if (display_id == Info::DemoScreen) {
			canvas[0] = tvg::SwCanvas::gen();
			canvas[0]->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			scene[0] = tvg::Scene::gen();

			// DarkGrey background
			bg = tvg::Shape::gen();
			bg->appendRect(0, 0, width, height);
			bg->fill(0x22, 0x22, 0x22, 0xFF);
			scene[0]->push(bg);

			// orange circle
			circle = tvg::Shape::gen();
			circle->appendCircle(25, 25, 10, 10);
			circle->fill(0xFF, 0x80, 0x20, 0x80);
			scene[0]->push(circle);

			// magenta circle
			oval = tvg::Shape::gen();
			oval->appendRect(5, 0, width - 5, 10);
			oval->fill(0x80, 0x00, 0xFF, 0x80);
			scene[0]->push(oval);

			auto scaling = float(width) / CoreHelper::base_element(DemoScreen).width_mm;
			scene[0]->scale(scaling);
			canvas[0]->push(scene[0]);

			needUpdateDisplay1 = true;
		}

		if (display_id == Info::DemoScreen2) {
			canvas[1] = tvg::SwCanvas::gen();
			canvas[1]->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			scene[1] = tvg::Scene::gen();

			// Light grey background
			bg2 = tvg::Shape::gen();
			bg2->appendRect(0, 0, width, height);
			bg2->fill(0xcc, 0xcc, 0xcc, 0xFF);
			scene[1]->push(bg2);

			rect2 = tvg::Shape::gen();
			rect2->appendRect(3, 3, 20, 20);
			rect2->fill(0xFF, 0x00, 0x00, 0xFF);
			scene[1]->push(rect2);

			auto scaling = float(width) / CoreHelper::base_element(DemoScreen2).width_mm;
			scene[1]->scale(scaling);
			canvas[1]->push(scene[1]);
		}
	}

	void hide_graphic_display(int display_id) override {
		if (display_id == Info::DemoScreen) {
			scene[0]->remove();
			canvas[0]->remove();
			delete canvas[0];
		}

		if (display_id == Info::DemoScreen) {
			scene[1]->remove();
			canvas[1]->remove();
			delete canvas[1];
		}
	}

	bool draw_graphic_display(int display_id) override {
		if (display_id == Info::DemoScreen) {
			// No need to update graphics if knob didn't move
			if (!needUpdateDisplay1)
				return false;
			needUpdateDisplay1 = false;

			circle->scale(clockDivideOffset + 0.5);
			oval->translate(0, clockDivideOffset * 10);
			canvas[0]->update(circle);
			canvas[0]->update(oval);

			canvas[0]->draw();
			canvas[0]->sync();

			return true;
		}

		if (display_id == Info::DemoScreen2) {
			if (anim_rot > 0) {
				// animate
				uint64_t now_ms = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;
				auto ticks_elapsed = now_ms - last_anim_tm;
				last_anim_tm = now_ms;
				anim_rot -= (float)ticks_elapsed / 20.f;
				if (anim_rot < 0)
					anim_rot = 0;

				rect2->rotate(anim_rot);
				canvas[1]->update(rect2);
			}
			canvas[1]->draw();
			canvas[1]->sync();
			return true;
		}

		return false;
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
				if (val != clockDivideOffset) {
					needUpdateDisplay1 = true;
					anim_rot = 90;
					last_anim_tm = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;
				}
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

	bool needUpdateDisplay1 = false;
	float anim_rot = 0;
	uint32_t last_anim_tm = 0;

	static constexpr float gateVoltage = 8.0f;
};

} // namespace MetaModule
