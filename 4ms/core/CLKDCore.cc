#include "CoreModules/CoreHelper.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/CLKD_info.hh"
#include "processors/tools/clockPhase.h"
#include "thorvg/thorvg/inc/thorvg.h"
#include "util/math.hh"

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

	std::array<tvg::Shape *, 8> circles;

	tvg::Shape *rect2;

	float scaling = 1.0f;

	~CLKDCore() = default;

	void show_graphic_display(int display_id, std::span<uint32_t> pix, unsigned width, lv_obj_t *) override {
		auto height = pix.size() / width;
		if (display_id == display_index<UpperScreen>()) {
			// Make a scene
			auto top_scene = tvg::Scene::gen();

			// DarkGrey background
			auto bg = tvg::Shape::gen();
			bg->appendRect(0, 0, width, height);
			bg->fill(0x22, 0x22, 0x22, 0xFF);
			top_scene->push(bg);

			// Grid of circles
			for (auto i = 0u; auto &c : circles) {
				c = tvg::Shape::gen();
				float x = int(i % 4) * 3.5f + 3;
				float y = int(i / 4) * 6 + 5;
				c->appendCircle(x, y, 1.5f, 1.5f);
				c->fill(0xFF, 0x80, 0x20, 0xFF);
				top_scene->push(c);
				i++;
			}

			///////////////// Boilerplate: push_scene(display_id, pix, width, top_scene, top_canvas);
			top_canvas = tvg::SwCanvas::gen();
			top_canvas->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			scaling = float(width) / base_element(UpperScreen).width_mm;
			top_scene->scale(scaling);
			top_canvas->push(top_scene);
			////////////
		}

		if (display_id == display_index<LowerScreen>()) {
			///////////////// Boilerplate: auto scene = begin_graphic_scene(display_id, pix, width);
			auto bottom_scene = tvg::Scene::gen();
			bottom_canvas = tvg::SwCanvas::gen();
			bottom_canvas->target(pix.data(), width, width, height, tvg::ColorSpace::ARGB8888);
			scaling = float(width) / base_element(LowerScreen).width_mm;
			bottom_scene->scale(scaling);
			////////////////////

			// Light grey background
			auto bg2 = tvg::Shape::gen();
			bg2->appendRect(0, 0, width, height);
			bg2->fill(0xcc, 0xcc, 0xcc, 0xFF);
			bottom_scene->push(bg2);

			rect2 = tvg::Shape::gen();
			rect2->appendRect(-10, -10, 20, 20);
			rect2->fill(0xFF, 0x00, 0x00, 0xFF);

			rect2->translate((float)width / 2, (float)height / 2);

			bottom_scene->push(rect2);

			/////////////////////// Boilerplate: end_graphic_scene(scene);
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

			auto div_idx = (unsigned)(clockDivideOffset * 7.99f);
			for (auto i = 0u; i < circles.size(); i++) {
				if (i == unsigned(cp.getWrappedPhase() * div_idx))
					circles[i]->fill(0xFF, 0xFF, 0, 0xFF);
				else if (i == div_idx)
					circles[i]->fill(0xFF, 0x80, 0x20, 0xFF);
				else
					circles[i]->fill(0xcc, 0xcc, 0xcc, 0xFF);

				top_canvas->update(circles[i]);
			}

			top_canvas->draw();
			top_canvas->sync();

			return true;
		}

		if (display_id == display_index<LowerScreen>()) {
			if (clockInit) {
				rect2->rotate(cp.getWrappedPhase() * 360);
				rect2->scale(scaling);
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

	static constexpr float gateVoltage = 8.0f;
};

} // namespace MetaModule
