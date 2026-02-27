#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct OctaveInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Octave"};
    static constexpr std::string_view description{"Octave Shifter"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Octave_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Octave.png"};

    using enum Coords;

    static constexpr std::array<Element, 4> Elements{{
		OctaveKnob{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Octave", ""}, 0.5f},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Input", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Out", ""}},
}};

    enum class Elem {
        OctaveKnob,
        CvIn,
        InputIn,
        Out,
    };

    // Legacy naming
    
    enum {
        KnobOctave, 
        NumKnobs,
    };
    
    
    enum {
        InputCv, 
        InputInput, 
        NumInJacks,
    };
    
    enum {
        OutputOut, 
        NumOutJacks,
    };

    // Only used in VCV, not in MM firmware
    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputInput, OutputOut}}};


};
} // namespace MetaModule
