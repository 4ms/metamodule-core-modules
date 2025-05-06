#pragma once
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct TSPInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"TSP"};
    static constexpr std::string_view description{"Triggered Sample Player"};
    static constexpr uint32_t width_hp = 14;
    static constexpr std::string_view svg_filename{"res/modules/TSP_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/TSP.png"};

    using enum Coords;

    static constexpr std::array<Element, 25> Elements{{
		MomentaryRGB7mm{{to_mm<72>(22.389), to_mm<72>(53.155), Center, "Play", ""}},
		MomentaryRGB7mm{{to_mm<72>(54.734), to_mm<72>(53.155), Center, "Loop", ""}},
		MomentaryRGB7mm{{to_mm<72>(86.556), to_mm<72>(53.155), Center, "Reverse", ""}},
		Davies1900hBlackKnob{{to_mm<72>(30.501), to_mm<72>(187.854), Center, "Pitch", ""}, 1.0f},
		Davies1900hBlackKnob{{to_mm<72>(96.64), to_mm<72>(187.854), Center, "StartPos", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(163.192), to_mm<72>(187.854), Center, "Length", ""}, 0.0f},
		WhiteMomentary7mm{{to_mm<72>(138.795), to_mm<72>(36.079), Center, "Prev Bank", ""}},
		WhiteMomentary7mm{{to_mm<72>(170.99), to_mm<72>(68.332), Center, "Next Sample", ""}},
		WhiteMomentary7mm{{to_mm<72>(170.99), to_mm<72>(34.942), Center, "Next Bank", ""}},
		WhiteMomentary7mm{{to_mm<72>(138.795), to_mm<72>(69.468), Center, "Prev Sample", ""}},
		STSPDisplay{{to_mm<72>(97.102), to_mm<72>(128.311), Center, "Screen", ""}},
		GateJackInput4ms{{to_mm<72>(38.546), to_mm<72>(235.576), Center, "Play Trig", ""}},
		AnalogJackInput4ms{{to_mm<72>(19.131), to_mm<72>(270.136), Center, "Pitch V/oct", ""}},
		GateJackInput4ms{{to_mm<72>(77.376), to_mm<72>(235.576), Center, "Reverse Trig", ""}},
		GateJackInput4ms{{to_mm<72>(117.62), to_mm<72>(235.576), Center, "Prev. Bank Trig", ""}},
		GateJackInput4ms{{to_mm<72>(156.45), to_mm<72>(235.826), Center, "Next Bank Trig", ""}},
		AnalogJackInput4ms{{to_mm<72>(57.961), to_mm<72>(270.136), Center, "Length CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(96.791), to_mm<72>(269.996), Center, "Start Pos CV", ""}},
		GateJackInput4ms{{to_mm<72>(135.621), to_mm<72>(270.136), Center, "Prev. Sample Trig", ""}},
		GateJackInput4ms{{to_mm<72>(174.451), to_mm<72>(270.136), Center, "Next Sample Trig", ""}},
		AnalogJackOutput4ms{{to_mm<72>(42.207), to_mm<72>(316.37), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(96.791), to_mm<72>(316.37), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(157.52), to_mm<72>(316.37), Center, "End Out", ""}},
		WhiteLight{{to_mm<72>(69.499), to_mm<72>(316.513), Center, "Play Light", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(19.794), to_mm<72>(100.749), Center, "LoadSample", ""}, 1, 0}, {"LoadSample"}},
}};

    enum class Elem {
        PlayButton,
        LoopButton,
        ReverseButton,
        PitchKnob,
        StartposKnob,
        LengthKnob,
        PrevBankButton,
        NextSampleButton,
        NextBankButton,
        PrevSampleButton,
        ScreenOut,
        PlayTrigIn,
        PitchV_OctIn,
        ReverseTrigIn,
        Prev_BankTrigIn,
        NextBankTrigIn,
        LengthCvIn,
        StartPosCvIn,
        Prev_SampleTrigIn,
        NextSampleTrigIn,
        LeftOut,
        RightOut,
        EndOut,
        PlayLight,
        LoadsampleAltParam,
    };

    // Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)
    
    enum {
        KnobPitch, 
        KnobStartpos, 
        KnobLength, 
        NumKnobs,
    };
    
    enum {
        SwitchPlay, 
        SwitchLoop, 
        SwitchReverse, 
        SwitchPrev_Bank, 
        SwitchNext_Sample, 
        SwitchNext_Bank, 
        SwitchPrev_Sample, 
        NumSwitches,
    };
    
    enum {
        InputPlay_Trig, 
        InputPitch_V_Oct, 
        InputReverse_Trig, 
        InputPrev__Bank_Trig, 
        InputNext_Bank_Trig, 
        InputLength_Cv, 
        InputStart_Pos_Cv, 
        InputPrev__Sample_Trig, 
        InputNext_Sample_Trig, 
        NumInJacks,
    };
    
    enum {
        OutputScreen, 
        OutputLeft_Out, 
        OutputRight_Out, 
        OutputEnd_Out, 
        NumOutJacks,
    };
    
    enum {
        LedPlay_Light, 
        NumDiscreteLeds,
    };
    
    enum {
        AltParamLoadsample, 
    };
};
} // namespace MetaModule
