#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct KPLSInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"KPLS"};
    static constexpr std::string_view description{"Karplus StrongVoice"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/KPLS_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/KPLS.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Pitch", ""}, 0.5f, 20.0, 400.0, "hz"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Decay", ""}, 0.25f, 200.0, 1000.0, "ms"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(143.15), Center, "Spread", ""}, 0.5f, 0.0, 100.0, "%"},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "V/Oct In", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Trigger In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.26), Center, "Audio Out", ""}},
}};

    enum class Elem {
        PitchKnob,
        DecayKnob,
        DetuneKnob,
        V_OctIn,
        TriggerIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobPitch, 
        KnobDecay, 
        KnobSpread, 
        NumKnobs,
    };
    
    
    enum {
        InputV_Oct_In, 
        InputTrigger_In, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
