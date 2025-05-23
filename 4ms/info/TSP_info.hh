#pragma once
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

    static constexpr std::array<Element, 15> Elements{{
		MomentaryRGB7mm{{to_mm<72>(26.196), to_mm<72>(161.096), Center, "Play", ""}},
		OrangeButton{{to_mm<72>(61.467), to_mm<72>(161.096), Center, "Loop", ""}},
		GateJackInput4ms{{to_mm<72>(25.131), to_mm<72>(210.048), Center, "Play Trig", ""}},
		GateJackInput4ms{{to_mm<72>(61.274), to_mm<72>(210.048), Center, "Loop Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(25.131), to_mm<72>(258.885), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(259.168), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(24.684), to_mm<72>(295.886), Center, "End Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(295.886), Center, "Position", ""}},
		DynamicGraphicDisplay{{to_mm<72>(9.234), to_mm<72>(60.753), TopLeft, "Waveform", "", to_mm<72>(67.938), to_mm<72>(66.398)}},
		TSPDisplay{{to_mm<72>(9.056), to_mm<72>(37.165), TopLeft, "Message", "", to_mm<72>(67.938), to_mm<72>(21.215)}},
		RedLight{{to_mm<72>(26.196), to_mm<72>(140.68), Center, "Busy Light", ""}},
		AltParamAction{{to_mm<72>(28.975), to_mm<72>(352.636), Center, "Load Sample...", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(74.72), to_mm<72>(352.629), Center, "Play Retrig Mode", ""}, 2, 0}, {"Retrigger", "Stop"}},
		AltParamContinuous{{to_mm<72>(63.524), to_mm<72>(352.716), Center, "WaveformZoom", ""}, 0.5f},
		AltParamChoiceLabeled{{{to_mm<72>(51.985), to_mm<72>(352.92), Center, "Prebuffer Amount", ""}, 5, 0}, {"Very Low", "Low", "Medium", "High", "Very High"}},
}};

    enum class Elem {
        PlayButton,
        LoopButton,
        PlayTrigIn,
        LoopGateIn,
        LeftOut,
        RightOut,
        EndOut,
        PositionOut,
        WaveformDisplay,
        MessageDisplay,
        BusyLight,
        LoadSampleAltParam,
        PlayRetrigModeAltParam,
        WaveformzoomAltParam,
        PrebufferAmountAltParam,
    };

};
} // namespace MetaModule
