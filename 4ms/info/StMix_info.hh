#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct StMixInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"StMix"};
    static constexpr std::string_view description{"Stereo Mixer"};
    static constexpr uint32_t width_hp = 18;
    static constexpr std::string_view svg_filename{"res/modules/StMix_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/StMix.png"};

    using enum Coords;

    static constexpr std::array<Element, 18> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(35.16), to_mm<72>(58.02), Center, "Ch. 1 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(98.12), to_mm<72>(58.02), Center, "Ch. 2 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(161.08), to_mm<72>(58.02), Center, "Ch. 3 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(224.04), to_mm<72>(58.02), Center, "Ch. 4 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(35.16), to_mm<72>(150.39), Center, "Ch. 1 Pan", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(98.12), to_mm<72>(150.17), Center, "Ch. 2 Pan", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(160.97), to_mm<72>(150.34), Center, "Ch. 3 Pan", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(224.04), to_mm<72>(150.17), Center, "Ch. 4 Pan", ""}, 0.5f},
		AnalogJackInput4ms{{to_mm<72>(34.78), to_mm<72>(222.7), Center, "Ch. 1 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(34.78), to_mm<72>(271.8), Center, "Ch. 1 Right In", ""}},
		AnalogJackInput4ms{{to_mm<72>(97.81), to_mm<72>(222.7), Center, "Ch. 2 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(97.81), to_mm<72>(271.8), Center, "Ch. 2 Right In", ""}},
		AnalogJackInput4ms{{to_mm<72>(160.85), to_mm<72>(222.7), Center, "Ch. 3 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(160.85), to_mm<72>(271.8), Center, "Ch. 3 Right In", ""}},
		AnalogJackInput4ms{{to_mm<72>(223.88), to_mm<72>(222.7), Center, "Ch. 4 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(223.88), to_mm<72>(271.8), Center, "Ch. 4 Right In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(138.33), to_mm<72>(322.18), Center, "Main Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(206.36), to_mm<72>(322.18), Center, "Main Right Out", ""}},
}};

    enum class Elem {
        Ch_1LevelKnob,
        Ch_2LevelKnob,
        Ch_3LevelKnob,
        Ch_4LevelKnob,
        Ch_1PanKnob,
        Ch_2PanKnob,
        Ch_3PanKnob,
        Ch_4PanKnob,
        Ch_1LeftIn,
        Ch_1RightIn,
        Ch_2LeftIn,
        Ch_2RightIn,
        Ch_3LeftIn,
        Ch_3RightIn,
        Ch_4LeftIn,
        Ch_4RightIn,
        MainLeftOut,
        MainRightOut,
    };

    // Legacy naming
    
    enum {
        KnobCh__1_Level, 
        KnobCh__2_Level, 
        KnobCh__3_Level, 
        KnobCh__4_Level, 
        KnobCh__1_Pan, 
        KnobCh__2_Pan, 
        KnobCh__3_Pan, 
        KnobCh__4_Pan, 
        NumKnobs,
    };
    
    
    enum {
        InputCh__1_Left_In, 
        InputCh__1_Right_In, 
        InputCh__2_Left_In, 
        InputCh__2_Right_In, 
        InputCh__3_Left_In, 
        InputCh__3_Right_In, 
        InputCh__4_Left_In, 
        InputCh__4_Right_In, 
        NumInJacks,
    };
    
    enum {
        OutputMain_Left_Out, 
        OutputMain_Right_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
