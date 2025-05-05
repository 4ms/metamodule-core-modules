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

    static constexpr std::array<Element, 26> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(96.64), to_mm<72>(195.354), Center, "StartPos", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(163.192), to_mm<72>(195.354), Center, "Length", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(30.501), to_mm<72>(195.354), Center, "Pitch", ""}, 1.0f},
		MomentaryRGB7mm{{to_mm<72>(71.996), to_mm<72>(77.315), Center, "Play", ""}},
		MomentaryRGB7mm{{to_mm<72>(119.046), to_mm<72>(77.315), Center, "Reverse", ""}},
		MomentaryArrowLeftButton{{to_mm<72>(140.803), to_mm<72>(39.83), Center, "Prev Sample", ""}},
		MomentaryArrowRightButton{{to_mm<72>(181.404), to_mm<72>(39.83), Center, "Next Sample", ""}},
		MomentaryArrowLeftButton{{to_mm<72>(12.852), to_mm<72>(39.83), Center, "Prev Bank", ""}},
		MomentaryArrowRightButton{{to_mm<72>(53.453), to_mm<72>(39.83), Center, "Next Bank", ""}},
		STSPDisplay{{to_mm<72>(97.102), to_mm<72>(131.957), Center, "Screen", ""}},
		GateJackInput4ms{{to_mm<72>(19.131), to_mm<72>(243.076), Center, "Play Trig", ""}},
		AnalogJackInput4ms{{to_mm<72>(57.961), to_mm<72>(243.636), Center, "Pitch V/oct A", ""}},
		GateJackInput4ms{{to_mm<72>(135.621), to_mm<72>(282.136), Center, "Prev. Sample Trig", ""}},
		GateJackInput4ms{{to_mm<72>(174.451), to_mm<72>(282.136), Center, "Next Sample Trig", ""}},
		GateJackInput4ms{{to_mm<72>(174.451), to_mm<72>(243.327), Center, "Next Bank Trig", ""}},
		GateJackInput4ms{{to_mm<72>(135.621), to_mm<72>(243.076), Center, "Prev. Bank Trig", ""}},
		GateJackInput4ms{{to_mm<72>(96.791), to_mm<72>(243.076), Center, "Reverse Trig", ""}},
		AnalogJackInput4ms{{to_mm<72>(19.131), to_mm<72>(282.136), Center, "Length CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(57.961), to_mm<72>(281.996), Center, "Start Pos CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(96.791), to_mm<72>(282.136), Center, "Sample CV", ""}},
		AnalogJackOutput4ms{{to_mm<72>(50.495), to_mm<72>(323.871), Center, "Out Left", ""}},
		AnalogJackOutput4ms{{to_mm<72>(96.791), to_mm<72>(323.871), Center, "Out Right", ""}},
		AnalogJackOutput4ms{{to_mm<72>(141.295), to_mm<72>(323.871), Center, "End Out", ""}},
		WhiteLight{{to_mm<72>(74.005), to_mm<72>(324.014), Center, "Play A Light", ""}},
		RedLight{{to_mm<72>(96.791), to_mm<72>(39.327), Center, "Busy", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(19.175), to_mm<72>(77.849), Center, "SampleDir", ""}, 2, 0}, {"OpenDir", "None"}},
}};

    enum class Elem {
        StartposKnob,
        LengthKnob,
        PitchKnob,
        PlayButton,
        ReverseButton,
        PrevSampleButton,
        NextSampleButton,
        PrevBankButton,
        NextBankButton,
        ScreenOut,
        PlayTrigIn,
        PitchV_OctAIn,
        Prev_SampleTrigIn,
        NextSampleTrigIn,
        NextBankTrigIn,
        Prev_BankTrigIn,
        ReverseTrigIn,
        LengthCvIn,
        StartPosCvIn,
        SampleCvIn,
        OutLeftOut,
        OutRightOut,
        EndOut,
        PlayALight,
        BusyLight,
        SampledirAltParam,
    };

    // Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)
    
    enum {
        KnobStartpos, 
        KnobLength, 
        KnobPitch, 
        NumKnobs,
    };
    
    enum {
        SwitchPlay, 
        SwitchReverse, 
        SwitchPrev_Sample, 
        SwitchNext_Sample, 
        SwitchPrev_Bank, 
        SwitchNext_Bank, 
        NumSwitches,
    };
    
    enum {
        InputPlay_Trig, 
        InputPitch_V_Oct_A, 
        InputPrev__Sample_Trig, 
        InputNext_Sample_Trig, 
        InputNext_Bank_Trig, 
        InputPrev__Bank_Trig, 
        InputReverse_Trig, 
        InputLength_Cv, 
        InputStart_Pos_Cv, 
        InputSample_Cv, 
        NumInJacks,
    };
    
    enum {
        OutputScreen, 
        OutputOut_Left, 
        OutputOut_Right, 
        OutputEnd_Out, 
        NumOutJacks,
    };
    
    enum {
        LedPlay_A_Light, 
        LedBusy, 
        NumDiscreteLeds,
    };
    
    enum {
        AltParamSampledir, 
    };
};
} // namespace MetaModule
