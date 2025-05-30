#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct DjembeInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Djembe"};
    static constexpr std::string_view description{"Djembe DrumVoice"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/Djembe_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Djembe.png"};

    using enum Coords;

    static constexpr std::array<Element, 10> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(31.83), to_mm<72>(57.85), Center, "Pitch", ""}, 0.25f, 20.0, 500.0, "hz"},
		Davies1900hBlackKnob{{to_mm<72>(83.37), to_mm<72>(57.85), Center, "Sharpness", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(31.83), to_mm<72>(119.09), Center, "Hit", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(83.37), to_mm<72>(119.09), Center, "Strike Amt In", ""}, 0.5f},
		AnalogJackInput4ms{{to_mm<72>(32.0), to_mm<72>(214.53), Center, "Pitch CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.2), to_mm<72>(214.53), Center, "Sharp CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(32.0), to_mm<72>(263.25), Center, "Hit CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.2), to_mm<72>(263.25), Center, "Strike CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(32.0), to_mm<72>(311.98), Center, "Trigger In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.2), to_mm<72>(311.98), Center, "Audio Out", ""}},
}};

    enum class Elem {
        PitchKnob,
        SharpnessKnob,
        HitKnob,
        StrikeAmtInKnob,
        PitchCvIn,
        SharpCvIn,
        HitCvIn,
        StrikeCvIn,
        TriggerIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobPitch, 
        KnobSharpness, 
        KnobHit, 
        KnobStrike_Amt_In, 
        NumKnobs,
    };
    
    
    enum {
        InputPitch_Cv_In, 
        InputSharp_Cv_In, 
        InputHit_Cv_In, 
        InputStrike_Cv_In, 
        InputTrigger_In, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
