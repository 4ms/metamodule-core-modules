#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct Switch41Info : ModuleInfoBase {
    static constexpr std::string_view slug{"Switch41"};
    static constexpr std::string_view description{"4 to 1 Switch"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/Switch41_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Switch41.png"};

    using enum Coords;

    static constexpr std::array<Element, 8> Elements{{
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(167.04), Center, "Ch. 1 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.3), to_mm<72>(167.04), Center, "Ch. 2 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(214.43), Center, "Ch. 3 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.3), to_mm<72>(214.43), Center, "Ch. 4 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(263.15), Center, "Clock", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.3), to_mm<72>(263.15), Center, "CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.76), to_mm<72>(311.88), Center, "Reset", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.3), to_mm<72>(311.88), Center, "Out", ""}},
}};

    enum class Elem {
        Ch_1In,
        Ch_2In,
        Ch_3In,
        Ch_4In,
        ClockIn,
        CvIn,
        ResetIn,
        Out,
    };

    // Legacy naming
    
    
    
    enum {
        InputCh__1_In, 
        InputCh__2_In, 
        InputCh__3_In, 
        InputCh__4_In, 
        InputClock, 
        InputCv, 
        InputReset, 
        NumInJacks,
    };
    
    enum {
        OutputOut, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
