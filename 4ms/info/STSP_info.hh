#pragma once
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct STSPInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"STSP"};
	static constexpr std::string_view description{"Stereo Triggered Sample Player"};
	static constexpr uint32_t width_hp = 20;
	static constexpr std::string_view svg_filename{"res/modules/STSP_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/STSP.png"};

	using enum Coords;

	static constexpr std::array<Element, 42> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(37.67), to_mm<72>(54.34), Center, "Pitch A", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(106.16), to_mm<72>(162.85), Center, "Sample A", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(37.67), to_mm<72>(110.0), Center, "Start Pos. A", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(37.67), to_mm<72>(166.48), Center, "Length A", ""}, 1.0f},
		MomentaryRGB7mm{{to_mm<72>(80.48), to_mm<72>(35.33), Center, "Play A", ""}},
		MomentaryRGB7mm{{to_mm<72>(37.24), to_mm<72>(208.82), Center, "Reverse A", ""}},
		MomentaryArrowLeftButton{{to_mm<72>(87.47), to_mm<72>(209.66), Center, "Prev Bank A", ""}},
		MomentaryArrowRightButton{{to_mm<72>(117.82), to_mm<72>(209.66), Center, "Next Bank A", ""}},
		Davies1900hBlackKnob{{to_mm<72>(252.89), to_mm<72>(54.27), Center, "Pitch B", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(184.44), to_mm<72>(162.7), Center, "Sample B", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(252.89), to_mm<72>(109.78), Center, "Start Pos. B", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(252.89), to_mm<72>(166.44), Center, "Length B", ""}, 1.0f},
		MomentaryRGB7mm{{to_mm<72>(213.03), to_mm<72>(34.27), Center, "Play B", ""}},
		MomentaryRGB7mm{{to_mm<72>(252.57), to_mm<72>(208.82), Center, "Reverse B", ""}},
		MomentaryArrowLeftButton{{to_mm<72>(171.83), to_mm<72>(209.66), Center, "Prev Bank B", ""}},
		MomentaryArrowRightButton{{to_mm<72>(202.18), to_mm<72>(209.66), Center, "Next Bank B", ""}},
		SmallButton{{to_mm<72>(112.67), to_mm<72>(42.74), Center, "Reload Samples", ""}},
		STSPDisplay{{24, 20, TopLeft, "Screen", "", 54, 25}},
		GateJackInput4ms{{to_mm<72>(27.2), to_mm<72>(283.45), Center, "Play Trig A", ""}},
		AnalogJackInput4ms{{to_mm<72>(73.91), to_mm<72>(284.01), Center, "Pitch V/oct A", ""}},
		GateJackInput4ms{{to_mm<72>(120.67), to_mm<72>(283.45), Center, "Reverse Trig A", ""}},
		GateJackInput4ms{{to_mm<72>(167.23), to_mm<72>(283.45), Center, "Reverse Trig B", ""}},
		AnalogJackInput4ms{{to_mm<72>(213.99), to_mm<72>(284.01), Center, "Pitch V/Oct B", ""}},
		GateJackInput4ms{{to_mm<72>(260.66), to_mm<72>(283.45), Center, "Play Trig B", ""}},
		AnalogJackInput4ms{{to_mm<72>(27.2), to_mm<72>(319.51), Center, "Length CV A", ""}},
		AnalogJackInput4ms{{to_mm<72>(73.91), to_mm<72>(319.37), Center, "Start Pos CV A", ""}},
		AnalogJackInput4ms{{to_mm<72>(120.67), to_mm<72>(319.51), Center, "Sample CV A", ""}},
		AnalogJackInput4ms{{to_mm<72>(167.23), to_mm<72>(319.45), Center, "Sample CV B", ""}},
		AnalogJackInput4ms{{to_mm<72>(213.94), to_mm<72>(319.45), Center, "Start Pos CV B", ""}},
		AnalogJackInput4ms{{to_mm<72>(260.66), to_mm<72>(319.45), Center, "Length CV B", ""}},
		AnalogJackOutput4ms{{to_mm<72>(27.2), to_mm<72>(248.15), Center, "Out A Left", ""}},
		AnalogJackOutput4ms{{to_mm<72>(73.91), to_mm<72>(248.15), Center, "Out A Right", ""}},
		AnalogJackOutput4ms{{to_mm<72>(213.99), to_mm<72>(248.15), Center, "Out B Left", ""}},
		AnalogJackOutput4ms{{to_mm<72>(260.66), to_mm<72>(248.15), Center, "Out B Right", ""}},
		AnalogJackOutput4ms{{to_mm<72>(118.0), to_mm<72>(248.15), Center, "End Out A", ""}},
		AnalogJackOutput4ms{{to_mm<72>(167.44), to_mm<72>(248.15), Center, "End Out B", ""}},
		RedGreenBlueLight{{to_mm<72>(187.0), to_mm<72>(209.66), Center, "Bank A Light", ""}},
		WhiteLight{{to_mm<72>(50.71), to_mm<72>(241.45), Center, "Play A Light", ""}},
		RedGreenBlueLight{{to_mm<72>(102.65), to_mm<72>(209.66), Center, "Bank B Light", ""}},
		WhiteLight{{to_mm<72>(237.55), to_mm<72>(241.45), Center, "Play B Light", ""}},
		RedLight{{to_mm<72>(165.56), to_mm<72>(42.74), Center, "Busy", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(143.335), to_mm<72>(207.855), Center, "SampleDir", ""}, 4, 1},
							  {"Root dir", "Samples-1", "Samples-2", "Samples-3"}},
	}};

	enum class Elem {
		PitchAKnob,
		SampleAKnob,
		StartPos_AKnob,
		LengthAKnob,
		PlayAButton,
		ReverseAButton,
		PrevBankAButton,
		NextBankAButton,
		PitchBKnob,
		SampleBKnob,
		StartPos_BKnob,
		LengthBKnob,
		PlayBButton,
		ReverseBButton,
		PrevBankBButton,
		NextBankBButton,
		ReloadSamplesButton,
		ScreenOut,
		PlayTrigAIn,
		PitchV_OctAIn,
		ReverseTrigAIn,
		ReverseTrigBIn,
		PitchV_OctBIn,
		PlayTrigBIn,
		LengthCvAIn,
		StartPosCvAIn,
		SampleCvAIn,
		SampleCvBIn,
		StartPosCvBIn,
		LengthCvBIn,
		OutALeftOut,
		OutARightOut,
		OutBLeftOut,
		OutBRightOut,
		EndOutAOut,
		EndOutBOut,
		BankALight,
		PlayALight,
		BankBLight,
		PlayBLight,
		BusyLight,
		SampledirAltParam,
	};

	// Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)

	enum {
		KnobPitch_A,
		KnobSample_A,
		KnobStart_Pos__A,
		KnobLength_A,
		KnobPitch_B,
		KnobSample_B,
		KnobStart_Pos__B,
		KnobLength_B,
		NumKnobs,
	};

	enum {
		SwitchPlay_A,
		SwitchReverse_A,
		SwitchPrev_Bank_A,
		SwitchNext_Bank_A,
		SwitchPlay_B,
		SwitchReverse_B,
		SwitchPrev_Bank_B,
		SwitchNext_Bank_B,
		SwitchReload_Samples,
		NumSwitches,
	};

	enum {
		InputPlay_Trig_A,
		InputPitch_V_Oct_A,
		InputReverse_Trig_A,
		InputReverse_Trig_B,
		InputPitch_V_Oct_B,
		InputPlay_Trig_B,
		InputLength_Cv_A,
		InputStart_Pos_Cv_A,
		InputSample_Cv_A,
		InputSample_Cv_B,
		InputStart_Pos_Cv_B,
		InputLength_Cv_B,
		NumInJacks,
	};

	enum {
		OutputScreen,
		OutputOut_A_Left,
		OutputOut_A_Right,
		OutputOut_B_Left,
		OutputOut_B_Right,
		OutputEnd_Out_A,
		OutputEnd_Out_B,
		NumOutJacks,
	};

	enum {
		LedBank_A_Light,
		LedPlay_A_Light,
		LedBank_B_Light,
		LedPlay_B_Light,
		LedBusy,
		NumDiscreteLeds,
	};

	enum {
		AltParamSampledir,
	};
};
} // namespace MetaModule
