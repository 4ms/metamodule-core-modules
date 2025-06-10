#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct PIInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"PI"};
    static constexpr std::string_view description{"Percussion Interface"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/PI_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/PI.png"};

    using enum Coords;

    static constexpr std::array<Element, 18> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(30.9), to_mm<72>(54.04), Center, "Sensitivity", ""}, 0.875f},
		Knob9mm{{to_mm<72>(86.53), to_mm<72>(46.58), Center, "Env Level", ""}, 1.0f},
		Knob9mm{{to_mm<72>(30.8), to_mm<72>(109.3), Center, "Sustain", ""}, 0.5f},
		Knob9mm{{to_mm<72>(86.53), to_mm<72>(97.941), Center, "Inverted Level", ""}, 1.0f},
		Knob9mm{{to_mm<72>(30.8), to_mm<72>(153.17), Center, "Env Decay", ""}, 0.5f},
		Toggle3posHoriz{{to_mm<72>(86.53), to_mm<72>(145.308), Center, "Gain", ""}, {"Low", "Med", "High"}},
		Toggle2posHoriz{{to_mm<72>(30.8), to_mm<72>(195.475), Center, "Env Mode", ""}, {"Follow", "Gen"}},
		AnalogJackOutput4ms{{to_mm<72>(86.52), to_mm<72>(196.113), Center, "Env. Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(19.83), to_mm<72>(221.47), Center, "Audio In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(86.52), to_mm<72>(248.361), Center, "Inverted Out", ""}},
		GateJackOutput4ms{{to_mm<72>(41.83), to_mm<72>(253.56), Center, "Gate Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(86.52), to_mm<72>(298.075), Center, "Audio Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(19.95), to_mm<72>(286.08), Center, "Env + Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(41.83), to_mm<72>(318.61), Center, "Env - Out", ""}},
		RedGreenBlueLight{{to_mm<72>(11.39), to_mm<72>(32.64), Center, "Sens. Light", ""}},
		WhiteLight{{to_mm<72>(50.37), to_mm<72>(230.46), Center, "Gate Light", ""}},
		BlueLight{{to_mm<72>(10.77), to_mm<72>(261.2), Center, "Env + Light", ""}},
		GreenLight{{to_mm<72>(50.37), to_mm<72>(292.62), Center, "Env - Light", ""}},
}};

    enum class Elem {
        SensitivityKnob,
        EnvLevelKnob,
        SustainKnob,
        InvertedLevelKnob,
        EnvDecayKnob,
        GainSwitch,
        EnvModeSwitch,
        Env_Out,
        AudioIn,
        InvertedOut,
        GateOut,
        AudioOut,
        EnvPOut,
        EnvNOut,
        Sens_Light,
        GateLight,
        EnvPLight,
        EnvNLight,
    };

    // Legacy naming
    
    enum {
        KnobSensitivity, 
        KnobEnv_Level, 
        KnobSustain, 
        KnobInverted_Level, 
        KnobEnv_Decay, 
        NumKnobs,
    };
    
    enum {
        SwitchGain, 
        SwitchEnv_Mode, 
        NumSwitches,
    };
    
    enum {
        InputAudio_In, 
        NumInJacks,
    };
    
    enum {
        OutputEnv__Out, 
        OutputInverted_Out, 
        OutputGate_Out, 
        OutputAudio_Out, 
        OutputEnv_P_Out, 
        OutputEnv_N_Out, 
        NumOutJacks,
    };
    
    enum {
        LedSens__Light, 
        LedGate_Light, 
        LedEnv_P_Light, 
        LedEnv_N_Light, 
        NumDiscreteLeds,
    };
    

};
} // namespace MetaModule
