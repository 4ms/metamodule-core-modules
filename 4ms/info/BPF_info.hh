#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct BPFInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"BPF"};
    static constexpr std::string_view description{"Band-Pass Filter"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/BPF_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/BPF.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Cutoff", ""}, 0.0f, 261.0, 1046.0, "hz"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Q", ""}, 0.0f, 1.0, 20.0, "x"},
		Toggle2posHoriz{{to_mm<72>(29.092), to_mm<72>(172.199), Center, "Mode", ""}, {"Standard", "Oberheim"}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "Cutoff CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Audio In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Bandpass Out", ""}},
}};

    enum class Elem {
        CutoffKnob,
        QKnob,
        ModeSwitch,
        CutoffCvIn,
        AudioIn,
        BandpassOut,
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
        OutputBandpass_Out,
        NumOutJacks,
    };

	// Only used in VCV, not in MM firmware
    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputAudio_In, OutputBandpass_Out}}};

};
} // namespace MetaModule
