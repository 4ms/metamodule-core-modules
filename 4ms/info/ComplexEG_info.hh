#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct ComplexEGInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"ComplexEG"};
    static constexpr std::string_view description{"Complex Envelope Generator"};
    static constexpr uint32_t width_hp = 15;
    static constexpr std::string_view svg_filename{"res/modules/ComplexEG_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/ComplexEG.png"};

    using enum Coords;

    static constexpr std::array<Element, 21> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(43.62), to_mm<72>(46.2), Center, "Attack", ""}, 0.125f, 1.0, 1000.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(108.12), to_mm<72>(45.53), Center, "Decay", ""}, 0.125f, 1.0, 1000.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(172.62), to_mm<72>(45.53), Center, "Release", ""}, 0.25f, 1.0, 1000.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(43.62), to_mm<72>(104.51), Center, "Attack Curve", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(108.12), to_mm<72>(104.51), Center, "Decay Curve", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(172.62), to_mm<72>(104.51), Center, "Release Curve", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(43.62), to_mm<72>(163.28), Center, "Sustain", ""}, 0.75f, 0.0, 8.0, "V"},
		Toggle2posHoriz{{to_mm<72>(109.084), to_mm<72>(164.749), Center, "Loop", ""}, {"Off", "On"}},
		Davies1900hBlackKnob{{to_mm<72>(172.52), to_mm<72>(163.23), Center, "Hold", ""}, 0.5f, 1.0, 1000.0, "ms"},
		AnalogJackInput4ms{{to_mm<72>(36.45), to_mm<72>(214.13), Center, "Gate In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.96), to_mm<72>(214.13), Center, "Attack CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(131.96), to_mm<72>(214.13), Center, "Decay CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(179.96), to_mm<72>(214.13), Center, "Release CV", ""}},
		AnalogJackOutput4ms{{to_mm<72>(36.45), to_mm<72>(265.71), Center, "Attack Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.96), to_mm<72>(265.71), Center, "Sustain CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(131.96), to_mm<72>(265.71), Center, "Hold CV", ""}},
		AnalogJackOutput4ms{{to_mm<72>(179.96), to_mm<72>(265.71), Center, "Decay Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(36.45), to_mm<72>(309.84), Center, "Release Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.96), to_mm<72>(309.84), Center, "Sustain Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(131.96), to_mm<72>(309.84), Center, "Hold Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(179.96), to_mm<72>(309.84), Center, "Env Out", ""}},
}};

    enum class Elem {
        AttackKnob,
        DecayKnob,
        ReleaseKnob,
        AttackCurveKnob,
        DecayCurveKnob,
        ReleaseCurveKnob,
        SustainKnob,
        LoopSwitch,
        HoldKnob,
        GateIn,
        AttackCvIn,
        DecayCvIn,
        ReleaseCvIn,
        AttackOut,
        SustainCvIn,
        HoldCvIn,
        DecayOut,
        ReleaseOut,
        SustainOut,
        HoldOut,
        EnvOut,
    };

    // Legacy naming
    
    enum {
        KnobAttack, 
        KnobDecay, 
        KnobRelease, 
        KnobAttack_Curve, 
        KnobDecay_Curve, 
        KnobRelease_Curve, 
        KnobSustain, 
        KnobHold, 
        NumKnobs,
    };
    
    enum {
        SwitchLoop, 
        NumSwitches,
    };
    
    enum {
        InputGate_In, 
        InputAttack_Cv, 
        InputDecay_Cv, 
        InputRelease_Cv, 
        InputSustain_Cv, 
        InputHold_Cv, 
        NumInJacks,
    };
    
    enum {
        OutputAttack_Out, 
        OutputDecay_Out, 
        OutputRelease_Out, 
        OutputSustain_Out, 
        OutputHold_Out, 
        OutputEnv_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
