#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct HPFInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"HPF"};
    static constexpr std::string_view description{"High-Pass Filter"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/HPF_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/HPF.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Cutoff", ""}, 0.5f, 130.0, 2093.0, "hz"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Q", ""}, 0.0f, 1.0, 20.0, "x"},
		Toggle2posHoriz{{to_mm<72>(29.269), to_mm<72>(170.57), Center, "Mode", ""}, {"Standard", "Korg"}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "Cutoff CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Audio In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Audio Out", ""}},
}};

    enum class Elem {
        CutoffKnob,
        QKnob,
        ModeSwitch,
        CutoffCvIn,
        AudioIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobCutoff, 
        KnobQ, 
        NumKnobs,
    };
    
    enum {
        SwitchMode, 
        NumSwitches,
    };
    
    enum {
        InputCutoff_Cv_In, 
        InputAudio_In, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out,
        NumOutJacks,
    };

	// Only used in VCV, not in MM firmware
    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputAudio_In, OutputAudio_Out}}};

};
} // namespace MetaModule
