#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct SISMInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"SISM"};
    static constexpr std::string_view description{"Shifting Inverting Signal Mingler"};
    static constexpr uint32_t width_hp = 12;
    static constexpr std::string_view svg_filename{"res/modules/SISM_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/SISM.png"};

    using enum Coords;

    static constexpr std::array<Element, 34> Elements{{
		Knob9mm{{to_mm<72>(64.25), to_mm<72>(46.64), Center, "Ch. 1 Scale", ""}, 1.0f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(107.45), to_mm<72>(46.64), Center, "Ch. 1 Shift", ""}, 0.5f, -10.0, 10.0, "v"},
		Knob9mm{{to_mm<72>(64.25), to_mm<72>(111.44), Center, "Ch. 2 Scale", ""}, 1.0f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(107.45), to_mm<72>(111.44), Center, "Ch. 2 Shift", ""}, 0.5f, -10.0, 10.0, "v"},
		Knob9mm{{to_mm<72>(64.25), to_mm<72>(176.24), Center, "Ch. 3 Scale", ""}, 1.0f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(107.45), to_mm<72>(176.24), Center, "Ch. 3 Shift", ""}, 0.5f, -10.0, 10.0, "v"},
		Knob9mm{{to_mm<72>(64.25), to_mm<72>(241.04), Center, "Ch. 4 Scale", ""}, 1.0f, -100.0, 100.0, "%"},
		Knob9mm{{to_mm<72>(107.45), to_mm<72>(241.04), Center, "Ch. 4 Shift", ""}, 0.5f, -10.0, 10.0, "v"},
		AnalogJackInput4ms{{to_mm<72>(21.77), to_mm<72>(52.84), Center, "Ch. 1 In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(150.52), to_mm<72>(52.9), Center, "Ch. 1 Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(21.77), to_mm<72>(117.64), Center, "Ch. 2 In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(150.52), to_mm<72>(117.7), Center, "Ch. 2 Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(21.77), to_mm<72>(182.44), Center, "Ch. 3 In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(150.52), to_mm<72>(182.5), Center, "Ch. 3 Out", ""}},
		AnalogJackInput4ms{{to_mm<72>(21.77), to_mm<72>(247.24), Center, "Ch. 4 In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(150.52), to_mm<72>(247.3), Center, "Ch. 4 Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(21.1), to_mm<72>(312.16), Center, "+ Slice", ""}},
		AnalogJackOutput4ms{{to_mm<72>(64.25), to_mm<72>(312.16), Center, "- Slice", ""}},
		AnalogJackOutput4ms{{to_mm<72>(107.45), to_mm<72>(312.16), Center, "Mix (SW)", ""}},
		AnalogJackOutput4ms{{to_mm<72>(150.7), to_mm<72>(312.16), Center, "Mix", ""}},
		RedLight{{to_mm<72>(146.69), to_mm<72>(29.5), Center, "LED - 1", ""}},
		BlueLight{{to_mm<72>(158.21), to_mm<72>(29.5), Center, "LED + 1", ""}},
		RedLight{{to_mm<72>(144.99), to_mm<72>(93.58), Center, "LED - 2", ""}},
		BlueLight{{to_mm<72>(156.5), to_mm<72>(93.58), Center, "LED + 2", ""}},
		RedLight{{to_mm<72>(145.02), to_mm<72>(158.45), Center, "LED - 3", ""}},
		BlueLight{{to_mm<72>(156.54), to_mm<72>(158.45), Center, "LED + 3", ""}},
		RedLight{{to_mm<72>(145.11), to_mm<72>(223.33), Center, "LED - 4", ""}},
		BlueLight{{to_mm<72>(156.63), to_mm<72>(223.33), Center, "LED + 4", ""}},
		BlueLight{{to_mm<72>(20.92), to_mm<72>(289.03), Center, "LED + Slice", ""}},
		RedLight{{to_mm<72>(64.26), to_mm<72>(288.6), Center, "LED - Slice", ""}},
		RedLight{{to_mm<72>(102.32), to_mm<72>(289.18), Center, "LED - Mix (SW)", ""}},
		BlueLight{{to_mm<72>(113.66), to_mm<72>(289.18), Center, "LED + Mix (SW)", ""}},
		RedLight{{to_mm<72>(145.66), to_mm<72>(289.03), Center, "LED - Mix", ""}},
		BlueLight{{to_mm<72>(157.0), to_mm<72>(289.03), Center, "LED + Mix", ""}},
}};

    enum class Elem {
        Ch_1ScaleKnob,
        Ch_1ShiftKnob,
        Ch_2ScaleKnob,
        Ch_2ShiftKnob,
        Ch_3ScaleKnob,
        Ch_3ShiftKnob,
        Ch_4ScaleKnob,
        Ch_4ShiftKnob,
        Ch_1In,
        Ch_1Out,
        Ch_2In,
        Ch_2Out,
        Ch_3In,
        Ch_3Out,
        Ch_4In,
        Ch_4Out,
        PSliceOut,
        NSliceOut,
        Mix_Sw_Out,
        MixOut,
        LedN1Light,
        LedP1Light,
        LedN2Light,
        LedP2Light,
        LedN3Light,
        LedP3Light,
        LedN4Light,
        LedP4Light,
        LedPSliceLight,
        LedNSliceLight,
        LedNMix_Sw_Light,
        LedPMix_Sw_Light,
        LedNMixLight,
        LedPMixLight,
    };

    // Legacy naming
    
    enum {
        KnobCh__1_Scale, 
        KnobCh__1_Shift, 
        KnobCh__2_Scale, 
        KnobCh__2_Shift, 
        KnobCh__3_Scale, 
        KnobCh__3_Shift, 
        KnobCh__4_Scale, 
        KnobCh__4_Shift, 
        NumKnobs,
    };
    
    
    enum {
        InputCh__1_In, 
        InputCh__2_In, 
        InputCh__3_In, 
        InputCh__4_In, 
        NumInJacks,
    };
    
    enum {
        OutputCh__1_Out, 
        OutputCh__2_Out, 
        OutputCh__3_Out, 
        OutputCh__4_Out, 
        OutputP_Slice, 
        OutputN_Slice, 
        OutputMix__Sw_, 
        OutputMix, 
        NumOutJacks,
    };
    
    enum {
        LedLed_N_1, 
        LedLed_P_1, 
        LedLed_N_2, 
        LedLed_P_2, 
        LedLed_N_3, 
        LedLed_P_3, 
        LedLed_N_4, 
        LedLed_P_4, 
        LedLed_P_Slice, 
        LedLed_N_Slice, 
        LedLed_N_Mix__Sw_, 
        LedLed_P_Mix__Sw_, 
        LedLed_N_Mix, 
        LedLed_P_Mix, 
        NumDiscreteLeds,
    };
    

};
} // namespace MetaModule
