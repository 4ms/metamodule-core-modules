#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct FMInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"FM"};
    static constexpr std::string_view description{"FM Oscillator"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/FM_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/FM.png"};

    using enum Coords;

    static constexpr std::array<Element, 14> Elements{{
		Knob9mm{{to_mm<72>(31.83), to_mm<72>(40.21), Center, "Pitch", ""}, 0.5f, 20.0, 20000.0, "hz"},
		Knob9mm{{to_mm<72>(83.37), to_mm<72>(40.21), Center, "Mix", ""}, 0.5f},
		Knob9mm{{to_mm<72>(31.83), to_mm<72>(83.35), Center, "Index", ""}, 0.625f},
		Knob9mm{{to_mm<72>(83.37), to_mm<72>(83.35), Center, "Index CV", ""}, 0.5f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(31.83), to_mm<72>(126.49), Center, "Ratio Coarse", ""}, 0.375f, 0.125, 16.0, "."},
		Knob9mm{{to_mm<72>(83.37), to_mm<72>(126.49), Center, "Ratio Fine", ""}, 0.0f, 0.5, 2.0, "."},
		Knob9mm{{to_mm<72>(31.83), to_mm<72>(169.63), Center, "Shape", ""}, 0.0f},
		Knob9mm{{to_mm<72>(83.37), to_mm<72>(169.63), Center, "Shape CV", ""}, 0.5f, -100.0, 100.0, "%"},
		AnalogJackInput4ms{{to_mm<72>(31.83), to_mm<72>(214.43), Center, "V/Oct Carrier", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.37), to_mm<72>(214.43), Center, "V/Oct Modulator", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.83), to_mm<72>(263.15), Center, "Mix CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.37), to_mm<72>(263.15), Center, "Index CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.83), to_mm<72>(311.88), Center, "Shape CV In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.37), to_mm<72>(311.88), Center, "Audio Out", ""}},
}};

    enum class Elem {
        PitchKnob,
        MixKnob,
        IndexKnob,
        IndexCvKnob,
        RatioCoarseKnob,
        RatioFineKnob,
        ShapeKnob,
        ShapeCvKnob,
        V_OctCarrierIn,
        V_OctModulatorIn,
        MixCvIn,
        IndexCvIn,
        ShapeCvIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobPitch, 
        KnobMix, 
        KnobIndex, 
        KnobIndex_Cv, 
        KnobRatio_Coarse, 
        KnobRatio_Fine, 
        KnobShape, 
        KnobShape_Cv, 
        NumKnobs,
    };
    
    
    enum {
        InputV_Oct_Carrier, 
        InputV_Oct_Modulator, 
        InputMix_Cv_In, 
        InputIndex_Cv_In, 
        InputShape_Cv_In, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
