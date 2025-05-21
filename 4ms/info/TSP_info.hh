#pragma once
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct TSPInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"TSP"};
    static constexpr std::string_view description{"Sample Player"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/TSP_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/TSP.png"};

    using enum Coords;

    static constexpr std::array<Element, 10> Elements{{
		MomentaryRGB7mm{{to_mm<72>(24.192), to_mm<72>(90.114), Center, "Play", ""}},
		MomentaryRGB7mm{{to_mm<72>(24.192), to_mm<72>(127.878), Center, "Loop", ""}},
		TSPDisplay{{to_mm<72>(24.346), to_mm<72>(52.915), Center, "Screen", ""}},
		GateJackInput4ms{{to_mm<72>(24.192), to_mm<72>(166.702), Center, "Play Trig", ""}},
		GateJackInput4ms{{to_mm<72>(24.192), to_mm<72>(209.801), Center, "Loop Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(11.645), to_mm<72>(291.034), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(37.956), to_mm<72>(291.317), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(24.192), to_mm<72>(253.125), Center, "End Out", ""}},
		RedLight{{to_mm<72>(3.142), to_mm<72>(85.582), Center, "Busy Light", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(22.954), to_mm<72>(37.064), Center, "LoadSample", ""}, 1, 0}, {"LoadSample"}},
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
