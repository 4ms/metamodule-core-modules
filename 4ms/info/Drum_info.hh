#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct DrumInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Drum"};
    static constexpr std::string_view description{"DrumVoice"};
    static constexpr uint32_t width_hp = 15;
    static constexpr std::string_view svg_filename{"res/modules/Drum_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Drum.png"};

    using enum Coords;

    static constexpr std::array<Element, 21> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(43.5), to_mm<72>(45.83), Center, "Pitch", ""}, 0.25f, 10.0, 1000.0, "hz"},
		Davies1900hBlackKnob{{to_mm<72>(108.0), to_mm<72>(45.83), Center, "Pitch Envelope", ""}, 0.25f, 10.0, 500.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(172.46), to_mm<72>(45.83), Center, "Pitch Amount", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(43.5), to_mm<72>(104.39), Center, "FM Ratio", ""}, 0.5f, 1.0, 16.0, "."},
		Davies1900hBlackKnob{{to_mm<72>(108.0), to_mm<72>(104.39), Center, "FM Envelope", ""}, 0.25f, 10.0, 8000.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(172.46), to_mm<72>(104.39), Center, "FM Amount", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(43.5), to_mm<72>(163.07), Center, "Tone Envelope", ""}, 0.25f, 10.0, 600.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(108.0), to_mm<72>(163.07), Center, "Noise Envelope", ""}, 0.0f, 100.0, 3000.0, "ms"},
		Davies1900hBlackKnob{{to_mm<72>(172.46), to_mm<72>(163.07), Center, "Noise Blend", ""}, 0.0f},
		AnalogJackInput4ms{{to_mm<72>(36.42), to_mm<72>(214.2), Center, "Trigger In", ""}},
		AnalogJackInput4ms{{to_mm<72>(84.7), to_mm<72>(214.2), Center, "V/Oct In", ""}},
		AnalogJackInput4ms{{to_mm<72>(132.98), to_mm<72>(214.2), Center, "Pitch Envelope CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(181.26), to_mm<72>(214.2), Center, "Pitch Amount CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(36.42), to_mm<72>(262.92), Center, "Ratio CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(84.7), to_mm<72>(262.92), Center, "FM Envelope CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(132.98), to_mm<72>(262.92), Center, "FM Amount CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(181.26), to_mm<72>(262.92), Center, "Tone Envelope CV In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(36.42), to_mm<72>(311.64), Center, "Inverted Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(84.7), to_mm<72>(311.64), Center, "Noise Envelope CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(132.98), to_mm<72>(311.64), Center, "Noise Blend CV In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(181.26), to_mm<72>(311.64), Center, "Audio Out", ""}},
}};

    enum class Elem {
        PitchKnob,
        PitchEnvelopeKnob,
        PitchAmountKnob,
        FmRatioKnob,
        FmEnvelopeKnob,
        FmAmountKnob,
        ToneEnvelopeKnob,
        NoiseEnvelopeKnob,
        NoiseBlendKnob,
        TriggerIn,
        V_OctIn,
        PitchEnvelopeCvIn,
        PitchAmountCvIn,
        RatioCvIn,
        FmEnvelopeCvIn,
        FmAmountCvIn,
        ToneEnvelopeCvIn,
        InvertedOut,
        NoiseEnvelopeCvIn,
        NoiseBlendCvIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobPitch, 
        KnobPitch_Envelope, 
        KnobPitch_Amount, 
        KnobFm_Ratio, 
        KnobFm_Envelope, 
        KnobFm_Amount, 
        KnobTone_Envelope, 
        KnobNoise_Envelope, 
        KnobNoise_Blend, 
        NumKnobs,
    };
    
    
    enum {
        InputTrigger_In, 
        InputV_Oct_In, 
        InputPitch_Envelope_Cv_In, 
        InputPitch_Amount_Cv_In, 
        InputRatio_Cv_In, 
        InputFm_Envelope_Cv_In, 
        InputFm_Amount_Cv_In, 
        InputTone_Envelope_Cv_In, 
        InputNoise_Envelope_Cv_In, 
        InputNoise_Blend_Cv_In, 
        NumInJacks,
    };
    
    enum {
        OutputInverted_Out, 
        OutputAudio_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
