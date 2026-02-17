#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct PitchShiftInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"PitchShift"};
    static constexpr std::string_view description{"Pitch Shifter"};
    static constexpr uint32_t width_hp = 7;
    static constexpr std::string_view svg_filename{"res/modules/PitchShift_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/PitchShift.png"};

    using enum Coords;

    static constexpr std::array<Element, 9> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(50.4), to_mm<72>(46.78), Center, "Coarse", ""}, 0.25f, -12.0, 12.0, "semitones"},
		Knob9mm{{to_mm<72>(50.4), to_mm<72>(94.75), Center, "Fine", ""}, 0.5f, -1.0, 1.0, "semitones"},
		Knob9mm{{to_mm<72>(50.4), to_mm<72>(139.5), Center, "Window", ""}, 0.0f, 0.42, 200.0, "ms"},
		Knob9mm{{to_mm<72>(50.4), to_mm<72>(184.25), Center, "Mix", ""}, 0.875f},
		AnalogJackInput4ms{{to_mm<72>(30.79), to_mm<72>(232.35), Center, "Audio In", ""}},
		AnalogJackInput4ms{{to_mm<72>(69.79), to_mm<72>(232.35), Center, "Pitch CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(30.79), to_mm<72>(281.45), Center, "Window CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(69.79), to_mm<72>(281.45), Center, "Mix CV", ""}},
		AnalogJackOutput4ms{{to_mm<72>(69.79), to_mm<72>(325.98), Center, "Audio Out", ""}},
}};

    enum class Elem {
        CoarseKnob,
        FineKnob,
        WindowKnob,
        MixKnob,
        AudioIn,
        PitchCvIn,
        WindowCvIn,
        MixCvIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobCoarse, 
        KnobFine, 
        KnobWindow, 
        KnobMix, 
        NumKnobs,
    };
    
    
    enum {
        InputAudio_In, 
        InputPitch_Cv, 
        InputWindow_Cv, 
        InputMix_Cv, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out,
        NumOutJacks,
    };

    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputAudio_In, OutputAudio_Out}}};

};
} // namespace MetaModule
