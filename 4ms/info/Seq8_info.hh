#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct Seq8Info : ModuleInfoBase {
    static constexpr std::string_view slug{"Seq8"};
    static constexpr std::string_view description{"8 Step Sequencer"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/Seq8_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Seq8.png"};

    using enum Coords;

    static constexpr std::array<Element, 12> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(32.75), to_mm<72>(47.1), Center, "Step 1", ""}, 0.25f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(84.28), to_mm<72>(47.1), Center, "Step 2", ""}, 0.5f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(32.75), to_mm<72>(104.19), Center, "Step 3", ""}, 0.875f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(84.28), to_mm<72>(104.19), Center, "Step 4", ""}, 0.25f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(32.75), to_mm<72>(163.5), Center, "Step 5", ""}, 0.5f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(84.28), to_mm<72>(163.5), Center, "Step 6", ""}, 0.875f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(32.75), to_mm<72>(222.27), Center, "Step 7", ""}, 1.0f, 0, 10, "V"},
		Davies1900hBlackKnob{{to_mm<72>(84.28), to_mm<72>(222.27), Center, "Step 8", ""}, 0.25f, 0, 10, "V"},
		AnalogJackInput4ms{{to_mm<72>(30.9), to_mm<72>(272.37), Center, "Clock", ""}},
		AnalogJackInput4ms{{to_mm<72>(84.57), to_mm<72>(272.37), Center, "Reset", ""}},
		AnalogJackOutput4ms{{to_mm<72>(31.0), to_mm<72>(314.05), Center, "Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(84.57), to_mm<72>(314.05), Center, "Out", ""}},
}};

    enum class Elem {
        Step1Knob,
        Step2Knob,
        Step3Knob,
        Step4Knob,
        Step5Knob,
        Step6Knob,
        Step7Knob,
        Step8Knob,
        ClockIn,
        ResetIn,
        GateOut,
        Out,
    };

    // Legacy naming
    
    enum {
        KnobStep_1, 
        KnobStep_2, 
        KnobStep_3, 
        KnobStep_4, 
        KnobStep_5, 
        KnobStep_6, 
        KnobStep_7, 
        KnobStep_8, 
        NumKnobs,
    };
    
    
    enum {
        InputClock, 
        InputReset, 
        NumInJacks,
    };
    
    enum {
        OutputGate, 
        OutputOut, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
