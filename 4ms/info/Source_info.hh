#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct SourceInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Source"};
    static constexpr std::string_view description{"Source"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Source_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Source.png"};

    using enum Coords;

    static constexpr std::array<Element, 4> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.72), Center, "Ch. 1 Offset", ""}, 0.5f, -10.0, 10.0, "v"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Ch. 2 Offset", ""}, 0.5f, -10.0, 10.0, "v"},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(265.11), Center, "Ch. 1 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.21), Center, "Ch. 2 Out", ""}},
}};

    enum class Elem {
        Ch_1OffsetKnob,
        Ch_2OffsetKnob,
        Ch_1Out,
        Ch_2Out,
    };

    // Legacy naming
    
    enum {
        KnobCh__1_Offset, 
        KnobCh__2_Offset, 
        NumKnobs,
    };
    
    
    
    enum {
        OutputCh__1_Out, 
        OutputCh__2_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
