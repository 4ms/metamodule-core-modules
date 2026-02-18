#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct PanInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Pan"};
    static constexpr std::string_view description{"Panner"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Pan_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Pan.png"};

    using enum Coords;

    static constexpr std::array<Element, 5> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Pan", ""}, 0.5f, -100.0, 100.0, "%"},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(168.66), Center, "Pan CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "Audio In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Ch. 1 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Ch. 2 Out", ""}},
}};

    enum class Elem {
        PanKnob,
        PanCvIn,
        AudioIn,
        Ch_1Out,
        Ch_2Out,
    };

    // Legacy naming
    
    enum {
        KnobPan, 
        NumKnobs,
    };
    
    
    enum {
        InputPan_Cv_In, 
        InputAudio_In, 
        NumInJacks,
    };
    
    enum {
        OutputCh__1_Out,
        OutputCh__2_Out,
        NumOutJacks,
    };

	// Not used:
    // static constexpr std::array<BypassRoute, 2> bypass_routes{{{InputAudio_In, OutputCh__1_Out}, {InputAudio_In, OutputCh__2_Out}}};

};
} // namespace MetaModule
