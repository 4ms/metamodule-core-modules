#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct RCDInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"RCD"};
    static constexpr std::string_view description{"Rotating Clock Divider"};
    static constexpr uint32_t width_hp = 10;
    static constexpr std::string_view svg_filename{"res/modules/RCD_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/RCD.png"};

    using enum Coords;

    static constexpr std::array<Element, 26> Elements{{
		Toggle2posHoriz{{to_mm<72>(108.8), to_mm<72>(93.345), Center, "Counting", ""}, {"Down", "Up"}, Toggle2posHoriz::State_t::RIGHT},
		Toggle2posHoriz{{to_mm<72>(108.8), to_mm<72>(133.445), Center, "Gate Trig", ""}, {"Gate", "Trig"}, Toggle2posHoriz::State_t::RIGHT},
		Toggle2posHoriz{{to_mm<72>(108.61), to_mm<72>(181.245), Center, "Max Div 1", ""}, {"32 or 64", "8 or 16"}, Toggle2posHoriz::State_t::RIGHT},
		Toggle2posHoriz{{to_mm<72>(108.61), to_mm<72>(220.965), Center, "Max Div 2", ""}, {"16 or 64", "8 or 32"}, Toggle2posHoriz::State_t::RIGHT},
		Toggle2posHoriz{{to_mm<72>(108.8), to_mm<72>(266.615), Center, "Spread", ""}, {"on", "off"}, Toggle2posHoriz::State_t::RIGHT},
		Toggle2posHoriz{{to_mm<72>(108.8), to_mm<72>(306.335), Center, "Auto-Reset", ""}, {"16", "off"}, Toggle2posHoriz::State_t::RIGHT},
		GateJackInput4ms{{to_mm<72>(48.19), to_mm<72>(56.31), Center, "Clk In", ""}},
		AnalogJackInput4ms{{to_mm<72>(92.61), to_mm<72>(56.31), Center, "Rotate", ""}},
		GateJackInput4ms{{to_mm<72>(123.98), to_mm<72>(56.31), Center, "Reset", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(87.93), Center, "/1", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(119.54), Center, "/2", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(151.16), Center, "/3", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(182.78), Center, "/4", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(214.39), Center, "/5", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(246.01), Center, "/6", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(277.62), Center, "/7", ""}},
		GateJackOutput4ms{{to_mm<72>(48.19), to_mm<72>(309.24), Center, "/8", ""}},
		WhiteLight{{to_mm<72>(19.98), to_mm<72>(56.31), Center, "LED in", ""}},
		BlueLight{{to_mm<72>(19.98), to_mm<72>(87.93), Center, "LED d1", ""}},
		RedLight{{to_mm<72>(19.98), to_mm<72>(119.54), Center, "LED d2", ""}},
		GreenLight{{to_mm<72>(19.98), to_mm<72>(151.16), Center, "LED d3", ""}},
		RedLight{{to_mm<72>(19.98), to_mm<72>(182.78), Center, "LED d4", ""}},
		BlueLight{{to_mm<72>(19.98), to_mm<72>(214.39), Center, "LED d5", ""}},
		GreenLight{{to_mm<72>(19.98), to_mm<72>(246.01), Center, "LED d6", ""}},
		BlueLight{{to_mm<72>(19.98), to_mm<72>(277.62), Center, "LED d7", ""}},
		RedLight{{to_mm<72>(19.98), to_mm<72>(309.24), Center, "LED d8", ""}},
}};

    enum class Elem {
        CountingSwitch,
        GateTrigSwitch,
        MaxDiv1Switch,
        MaxDiv2Switch,
        SpreadSwitch,
        AutoNResetSwitch,
        ClkIn,
        RotateIn,
        ResetIn,
        _1Out,
        _2Out,
        _3Out,
        _4Out,
        _5Out,
        _6Out,
        _7Out,
        _8Out,
        LedInLight,
        LedD1Light,
        LedD2Light,
        LedD3Light,
        LedD4Light,
        LedD5Light,
        LedD6Light,
        LedD7Light,
        LedD8Light,
    };

    // Legacy naming
    
    
    enum {
        SwitchCounting, 
        SwitchGate_Trig, 
        SwitchMax_Div_1, 
        SwitchMax_Div_2, 
        SwitchSpread, 
        SwitchAutoNReset, 
        NumSwitches,
    };
    
    enum {
        InputClk_In, 
        InputRotate, 
        InputReset, 
        NumInJacks,
    };
    
    enum {
        Output_1, 
        Output_2, 
        Output_3, 
        Output_4, 
        Output_5, 
        Output_6, 
        Output_7, 
        Output_8, 
        NumOutJacks,
    };
    
    enum {
        LedLed_In, 
        LedLed_D1, 
        LedLed_D2, 
        LedLed_D3, 
        LedLed_D4, 
        LedLed_D5, 
        LedLed_D6, 
        LedLed_D7, 
        LedLed_D8, 
        NumDiscreteLeds,
    };
    

};
} // namespace MetaModule
