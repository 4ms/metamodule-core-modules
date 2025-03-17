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

class CLKDCore : public CoreProcessor, CoreHelper<CLKDInfo> {
	using Info = CLKDInfo;
	using ThisCore = CLKDCore;
	using enum CLKDInfo::Elem;

public:
	CLKDCore() = default;

	tvg::SwCanvas *top_canvas = nullptr;
	tvg::SwCanvas *bottom_canvas = nullptr;

	tvg::Scene *top_scene;
	tvg::Scene *bottom_scene;

	tvg::Shape *bg;
	tvg::Shape *circle;

	tvg::Shape *bg2;
	tvg::Shape *rect2;

	~CLKDCore() = default;

	void show_graphic_display(int display_id, std::span<uint32_t> pix, uint16_t width, uint16_t height) override {
		if (display_id == display_index<UpperScreen>()) {
			top_canvas = tvg::SwCanvas::gen();
			top_canvas->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			top_scene = tvg::Scene::gen();
			/////////////////

			// DarkGrey background
			bg = tvg::Shape::gen();
			bg->appendRect(0, 0, width, height);
			bg->fill(0x22, 0x22, 0x22, 0xFF);
			top_scene->push(bg);

			// orange circle-ish shape
			circle = tvg::Shape::gen();
			circle->appendCircle(0, 0, 10, 10);
			circle->appendRect(-12, -2, 24, 4);
			circle->appendRect(-2, -12, 4, 24);
			circle->translate((float)width / 2, (float)height / 2);
			circle->fill(0xFF, 0x80, 0x20, 0xFF);

			top_scene->push(circle);

			////////////
			auto scaling = float(width) / base_element(UpperScreen).width_mm;
			top_scene->scale(scaling);
			top_canvas->push(top_scene);

			needUpdateDisplay1 = true;
		}

		if (display_id == display_index<LowerScreen>()) {
			bottom_canvas = tvg::SwCanvas::gen();
			bottom_canvas->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			bottom_scene = tvg::Scene::gen();

			// Light grey background
			bg2 = tvg::Shape::gen();
			bg2->appendRect(0, 0, width, height);
			bg2->fill(0xcc, 0xcc, 0xcc, 0xFF);
			bottom_scene->push(bg2);

			rect2 = tvg::Shape::gen();
			rect2->appendRect(-10, -10, 20, 20);
			rect2->translate(20, 20);
			rect2->fill(0xFF, 0x00, 0x00, 0xFF);

			auto cutout = tvg::Shape::gen();
			cutout->appendRect(-5, -5, 10, 10);
			cutout->fill(0xcc, 0xcc, 0xcc, 0xFF);
			rect2->mask(cutout, tvg::MaskMethod::Subtract);

			bottom_scene->push(rect2);

			////////////////

			auto scaling = float(width) / base_element(LowerScreen).width_mm;
			bottom_scene->scale(scaling);

			bottom_canvas->push(bottom_scene);
		}
	}

	void hide_graphic_display(int display_id) override {
		if (display_id == display_index<UpperScreen>()) {
			if (top_canvas) {
				delete top_canvas;
				top_canvas = nullptr;
			}
		}

		if (display_id == display_index<LowerScreen>()) {
			if (bottom_canvas) {
				delete bottom_canvas;
				bottom_canvas = nullptr;
			}
		}
	}

	bool draw_graphic_display(int display_id) override {
		if (display_id == display_index<UpperScreen>()) {
			// No need to update graphics if knob didn't move
			if (!needUpdateDisplay1)
				return false;
			needUpdateDisplay1 = false;

			// circle->scale(clockDivideOffset + 0.5);
			top_canvas->update(circle);

			top_canvas->draw();
			top_canvas->sync();

			return true;
		}

		if (display_id == display_index<LowerScreen>()) {
			if (clockInit) {
				rect2->rotate(cp.getWrappedPhase() * 360);
				bottom_canvas->update(rect2);

				bottom_canvas->draw();
				bottom_canvas->sync();
			}
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
		if (param_id == param_index<DivideKnob>()) {
			if (val != clockDivideOffset) {
				needUpdateDisplay1 = true;
			}
			clockDivideOffset = val;
			update_divider();
		}
	}

	float get_param(int param_id) const override {
		if (param_id == param_index<DivideKnob>())
			return clockDivideOffset;
		else
			return 0;
	}

	void set_input(int input_id, float val) override {
		if (input_id == input_index<ClkIn>()) {
			cp.updateClock(val);
			clockInit = true;

		} else if (input_id == input_index<CvIn>()) {
			float tmp = val / CvRangeVolts;
			if (tmp != clockDivideCV) {
				clockDivideCV = tmp;
				update_divider();
			}
		}
	}

	float get_output(int output_id) const override {
		return clockOutput;
	}

	void set_samplerate(float sr) override {
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

	static constexpr float gateVoltage = 8.0f;
};

} // namespace MetaModule
