#pragma once
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/CoreHelper.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct CLKDInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"CLKD"};
	static constexpr std::string_view description{"Clock Divider"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view svg_filename{"res/modules/CLKD_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/CLKD.png"};

	using enum Coords;

	static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Divide", ""}, 0.0f},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "CV", ""}},
		GateJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Clk In", ""}},
		GateJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Clk Out", ""}},
		DynamicGraphicDisplay{{{{to_mm<72>(5), to_mm<72>(70), TopLeft, "Demo", "", to_mm<72>(50), to_mm<72>(45)}}}},
		DynamicGraphicDisplay{{{{to_mm<72>(5), to_mm<72>(120), TopLeft, "Demo2", "", to_mm<72>(50), to_mm<72>(80)}}}},
	}};

	enum class Elem { DivideKnob, CvIn, ClkIn, ClkOut, DemoScreen, DemoScreen2 };

	enum {
		KnobDivide = CoreHelper<CLKDInfo>::param_index<Elem::DivideKnob>(),
		NumKnobs = CoreHelper<CLKDInfo>::count().num_params,
	};

	enum {
		InputCv = CoreHelper<CLKDInfo>::input_index<Elem::CvIn>(),
		InputClk_In = CoreHelper<CLKDInfo>::input_index<Elem::ClkIn>(),
		NumInJacks = CoreHelper<CLKDInfo>::count().num_inputs,
	};

	enum {
		OutputClk_Out = CoreHelper<CLKDInfo>::output_index<Elem::ClkOut>(),
		NumOutJacks = CoreHelper<CLKDInfo>::count().num_outputs,
	};

	enum Lights {
		DemoScreen = CoreHelper<CLKDInfo>::first_light_index<Elem::DemoScreen>(),
		DemoScreen2 = CoreHelper<CLKDInfo>::first_light_index<Elem::DemoScreen2>(),
		NumLights = CoreHelper<CLKDInfo>::count().num_lights,
	};
};
} // namespace MetaModule
