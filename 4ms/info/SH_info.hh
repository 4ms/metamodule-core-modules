#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct SHInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"SH"};
    static constexpr std::string_view description{"2 Ch. Sample and Hold"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/SH_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/SH.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(72.35), Center, "Ch. 1 Sample In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(120.54), Center, "Ch. 1 Clock In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(168.72), Center, "Ch. 2 Sample In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "Ch. 2 Clock In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Ch. 1 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Ch. 2 Out", ""}},
}};

    enum class Elem {
        Ch_1SampleIn,
        Ch_1ClockIn,
        Ch_2SampleIn,
        Ch_2ClockIn,
        Ch_1Out,
        Ch_2Out,
    };

    // Legacy naming
    
    
    
    enum {
        InputCh__1_Sample_In, 
        InputCh__1_Clock_In, 
        InputCh__2_Sample_In, 
        InputCh__2_Clock_In, 
        NumInJacks,
    };
    
    enum {
        OutputCh__1_Out, 
        OutputCh__2_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
