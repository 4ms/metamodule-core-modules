#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct L4Info : ModuleInfoBase {
    static constexpr std::string_view slug{"L4"};
    static constexpr std::string_view description{"Listen Four"};
    static constexpr uint32_t width_hp = 10;
    static constexpr std::string_view svg_filename{"res/modules/L4_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/L4.png"};

    using enum Coords;

    static constexpr std::array<Element, 24> Elements{{
		Knob9mm{{to_mm<72>(62.33), to_mm<72>(46.17), Center, "Ch. 1 Pan", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(111.89), to_mm<72>(59.09), Center, "Ch. 1 Level", ""}, 0.875f},
		Knob9mm{{to_mm<72>(62.33), to_mm<72>(103.86), Center, "Ch. 2 Pan", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(111.89), to_mm<72>(116.69), Center, "Ch. 2 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(111.89), to_mm<72>(174.29), Center, "Ch. 3 Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(111.89), to_mm<72>(232.15), Center, "Ch. 4 Level", ""}, 0.875f},
		Toggle2posHoriz{{to_mm<72>(67.265), to_mm<72>(258.19), Center, "Output Level", ""}, {"Modular", "Line Level"}},
		Knob9mm{{to_mm<72>(23.27), to_mm<72>(318.85), Center, "Headphone Level", ""}, 0.875f},
		Davies1900hBlackKnob{{to_mm<72>(71.7), to_mm<72>(302.94), Center, "Main Level", ""}, 0.875f},
		AnalogJackInput4ms{{to_mm<72>(23.31), to_mm<72>(46.17), Center, "Ch. 1 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(23.31), to_mm<72>(103.92), Center, "Ch. 2 In", ""}},
		AnalogJackInput4ms{{to_mm<72>(23.31), to_mm<72>(161.52), Center, "Ch. 3 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(62.91), to_mm<72>(161.52), Center, "Ch. 3 Right In", ""}},
		AnalogJackInput4ms{{to_mm<72>(23.31), to_mm<72>(219.12), Center, "Ch. 4 Left In", ""}},
		AnalogJackInput4ms{{to_mm<72>(62.91), to_mm<72>(219.12), Center, "Ch. 4 Right In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(23.64), to_mm<72>(266.4), Center, "Headphone Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(122.02), to_mm<72>(282.83), Center, "Main Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(122.02), to_mm<72>(326.07), Center, "Main Right Out", ""}},
		RedGreenBlueLight{{to_mm<72>(87.38), to_mm<72>(46.17), Center, "Ch. 1 Level LED", ""}},
		RedGreenBlueLight{{to_mm<72>(87.38), to_mm<72>(105.8), Center, "Ch. 2 Level LED", ""}},
		RedGreenBlueLight{{to_mm<72>(87.38), to_mm<72>(163.03), Center, "Ch. 3 Level LED", ""}},
		RedGreenBlueLight{{to_mm<72>(87.38), to_mm<72>(221.73), Center, "Ch. 4 Level LED", ""}},
		RedGreenBlueLight{{to_mm<72>(98.07), to_mm<72>(284.23), Center, "Main Out Left LED", ""}},
		RedGreenBlueLight{{to_mm<72>(98.07), to_mm<72>(316.53), Center, "Main Out Right LED", ""}},
}};

    enum class Elem {
        Ch_1PanKnob,
        Ch_1LevelKnob,
        Ch_2PanKnob,
        Ch_2LevelKnob,
        Ch_3LevelKnob,
        Ch_4LevelKnob,
        OutputLevelSwitch,
        HeadphoneLevelKnob,
        MainLevelKnob,
        Ch_1In,
        Ch_2In,
        Ch_3LeftIn,
        Ch_3RightIn,
        Ch_4LeftIn,
        Ch_4RightIn,
        HeadphoneOut,
        MainLeftOut,
        MainRightOut,
        Ch_1LevelLedLight,
        Ch_2LevelLedLight,
        Ch_3LevelLedLight,
        Ch_4LevelLedLight,
        MainOutLeftLedLight,
        MainOutRightLedLight,
    };

    // Legacy naming
    
    enum {
        KnobCh__1_Pan, 
        KnobCh__1_Level, 
        KnobCh__2_Pan, 
        KnobCh__2_Level, 
        KnobCh__3_Level, 
        KnobCh__4_Level, 
        KnobHeadphone_Level, 
        KnobMain_Level, 
        NumKnobs,
    };
    
    enum {
        SwitchOutput_Level, 
        NumSwitches,
    };
    
    enum {
        InputCh__1_In, 
        InputCh__2_In, 
        InputCh__3_Left_In, 
        InputCh__3_Right_In, 
        InputCh__4_Left_In, 
        InputCh__4_Right_In, 
        NumInJacks,
    };
    
    enum {
        OutputHeadphone_Out, 
        OutputMain_Left_Out, 
        OutputMain_Right_Out, 
        NumOutJacks,
    };
    
    enum {
        LedCh__1_Level_Led, 
        LedCh__2_Level_Led, 
        LedCh__3_Level_Led, 
        LedCh__4_Level_Led, 
        LedMain_Out_Left_Led, 
        LedMain_Out_Right_Led, 
        NumDiscreteLeds,
    };
    

};
} // namespace MetaModule
