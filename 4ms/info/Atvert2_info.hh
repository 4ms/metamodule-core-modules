#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct Atvert2Info : ModuleInfoBase {
    static constexpr std::string_view slug{"Atvert2"};
    static constexpr std::string_view description{"Dual Attenuverter"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Atvert2_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Atvert2.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.9), to_mm<72>(46.86), Center, "Ch. 1", ""}, 1.0f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Ch. 2", ""}, 1.0f, -100.0, 100.0, "%"},
		AnalogJackInput4ms{{to_mm<72>(28.88), to_mm<72>(168.72), Center, "Ch. 1 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.88), to_mm<72>(216.87), Center, "Ch. 2 In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.88), to_mm<72>(265.02), Center, "Ch. 1 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.88), to_mm<72>(313.16), Center, "Ch. 2 Out", ""}},
}};

    enum class Elem {
        Ch_1Knob,
        Ch_2Knob,
        Ch_1In,
        Ch_2In,
        Ch_1Out,
        Ch_2Out,
    };

    // Legacy naming
    
    enum {
        KnobCh__1, 
        KnobCh__2, 
        NumKnobs,
    };
    
    
    enum {
        InputCh__1_In, 
        InputCh__2_In, 
        NumInJacks,
    };
    
    enum {
        OutputCh__1_Out, 
        OutputCh__2_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
