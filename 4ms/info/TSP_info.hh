#pragma once
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct TSPInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"TSP"};
	static constexpr std::string_view description{"Sample Player"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view svg_filename{"res/modules/TSP_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/TSP.png"};

	using enum Coords;

	static constexpr std::array<Element, 10> Elements{{
		MomentaryRGB7mm{{to_mm<72>(26.196), to_mm<72>(154.699), Center, "Play", ""}},
		MomentaryRGB7mm{{to_mm<72>(62.339), to_mm<72>(154.65), Center, "Loop", ""}},
		TSPDisplay{{to_mm<72>(10), to_mm<72>(34), TopLeft, "Screen", "", to_mm<72>(65), to_mm<72>(91)}},
		GateJackInput4ms{{to_mm<72>(25.131), to_mm<72>(203.65), Center, "Play Trig", ""}},
		GateJackInput4ms{{to_mm<72>(61.274), to_mm<72>(203.65), Center, "Loop Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(25.131), to_mm<72>(291.286), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(291.569), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(43.203), to_mm<72>(250.377), Center, "End Out", ""}},
		RedLight{{to_mm<72>(43.203), to_mm<72>(140.68), Center, "Busy Light", ""}},
		AltParamAction{to_mm<72>(42.136), to_mm<72>(356.476), Center, "Load Sample...", "Load Sample..."},
	}};

	enum class Elem {
		PlayButton,
		LoopButton,
		ScreenOut,
		PlayTrigIn,
		LoopGateIn,
		LeftOut,
		RightOut,
		EndOut,
		BusyLight,
		LoadsampleAltParam,
	};

	// Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)

	enum {
		SwitchPlay,
		SwitchLoop,
		NumSwitches,
	};

	enum {
		InputPlay_Trig,
		InputLoop_Gate,
		NumInJacks,
	};

	enum {
		OutputScreen,
		OutputLeft_Out,
		OutputRight_Out,
		OutputEnd_Out,
		NumOutJacks,
	};

	enum {
		LedBusy_Light,
		NumDiscreteLeds,
	};

	enum {
		AltParamLoadsample,
	};
};
} // namespace MetaModule
