#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct Switch14Info : ModuleInfoBase {
    static constexpr std::string_view slug{"Switch14"};
    static constexpr std::string_view description{"1 to 4 Switch"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/Switch14_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Switch14.png"};

    using enum Coords;

    static constexpr std::array<Element, 8> Elements{{
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(167.04), Center, "Signal In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.3), to_mm<72>(167.04), Center, "Reset", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(214.43), Center, "Clock", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.3), to_mm<72>(214.43), Center, "CV", ""}},
		AnalogJackOutput4ms{{to_mm<72>(31.76), to_mm<72>(263.15), Center, "Ch. 1 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.3), to_mm<72>(263.15), Center, "Ch. 2 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(31.76), to_mm<72>(311.88), Center, "Ch. 3 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.3), to_mm<72>(311.88), Center, "Ch. 4 Out", ""}},
}};

    enum class Elem {
        SignalIn,
        ResetIn,
        ClockIn,
        CvIn,
        Ch_1Out,
        Ch_2Out,
        Ch_3Out,
        Ch_4Out,
    };

    // Legacy naming
    
    
    
    enum {
        InputSignal_In, 
        InputReset, 
        InputClock, 
        InputCv, 
        NumInJacks,
    };
    
    enum {
        OutputCh__1_Out, 
        OutputCh__2_Out, 
        OutputCh__3_Out, 
        OutputCh__4_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
