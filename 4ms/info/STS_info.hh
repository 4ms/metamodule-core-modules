#pragma once
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct STSInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"STS"};
	static constexpr std::string_view description{"Stereo Triggered Sampler"};
	static constexpr uint32_t width_hp = 20;
	static constexpr std::string_view svg_filename{"res/modules/STS_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/STS.png"};

	using enum Coords;

	static constexpr std::array<Element, 40> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(37.52), to_mm<72>(54.34), Center, "Pitch L", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(95.87), to_mm<72>(90.17), Center, "Sample L", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(37.24), to_mm<72>(123.0), Center, "Start Pos. L", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(69.36), to_mm<72>(189.23), Center, "Length L", ""}, 1.0f},
		MomentaryRGB7mm{{to_mm<72>(81.91), to_mm<72>(49.62), Center, "Play L", ""}},
		MomentaryRGB7mm{{to_mm<72>(122.85), to_mm<72>(41.6), Center, "Bank L", ""}},
		MomentaryRGB7mm{{to_mm<72>(21.08), to_mm<72>(179.74), Center, "Reverse L", ""}},
		Davies1900hBlackKnob{{to_mm<72>(249.91), to_mm<72>(54.27), Center, "Pitch R", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(191.25), to_mm<72>(90.02), Center, "Sample R", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(249.6), to_mm<72>(122.95), Center, "Start Pos. R", ""}, 0.0f},
		Davies1900hBlackKnob{{to_mm<72>(218.16), to_mm<72>(189.23), Center, "Length R", ""}, 1.0f},
		MomentaryRGB7mm{{to_mm<72>(205.55), to_mm<72>(49.77), Center, "Play R", ""}},
		MomentaryRGB7mm{{to_mm<72>(164.37), to_mm<72>(41.88), Center, "Bank R", ""}},
		MomentaryRGB7mm{{to_mm<72>(266.32), to_mm<72>(179.74), Center, "Reverse R", ""}},
		Davies1900hBlackKnob{{to_mm<72>(143.45), to_mm<72>(183.69), Center, "Rec. Sample", ""}, 0.0f},
		MomentaryRGB7mm{{to_mm<72>(127.63), to_mm<72>(139.65), Center, "REC", ""}},
		MomentaryRGB7mm{{to_mm<72>(159.13), to_mm<72>(139.65), Center, "Rec. Bank", ""}},
		GateJackInput4ms{{to_mm<72>(27.2), to_mm<72>(283.45), Center, "Play Trig L", ""}},
		AnalogJackInput4ms{{to_mm<72>(66.74), to_mm<72>(248.15), Center, "1V/Oct L", ""}},
		GateJackInput4ms{{to_mm<72>(260.66), to_mm<72>(283.45), Center, "Play Trig R", ""}},
		AnalogJackInput4ms{{to_mm<72>(220.3), to_mm<72>(248.15), Center, "1V/Oct R", ""}},
		GateJackInput4ms{{to_mm<72>(110.74), to_mm<72>(283.45), Center, "Reverse Trig L", ""}},
		GateJackInput4ms{{to_mm<72>(177.15), to_mm<72>(283.45), Center, "Reverse Jack R", ""}},
		AnalogJackInput4ms{{to_mm<72>(27.2), to_mm<72>(319.51), Center, "Length CV L", ""}},
		AnalogJackInput4ms{{to_mm<72>(73.91), to_mm<72>(319.37), Center, "Start Pos CV L", ""}},
		AnalogJackInput4ms{{to_mm<72>(120.67), to_mm<72>(319.51), Center, "Sample CV L", ""}},
		AnalogJackInput4ms{{to_mm<72>(167.23), to_mm<72>(319.45), Center, "Sample CV R", ""}},
		AnalogJackInput4ms{{to_mm<72>(213.99), to_mm<72>(319.45), Center, "Start Pos CV R", ""}},
		AnalogJackInput4ms{{to_mm<72>(260.66), to_mm<72>(319.45), Center, "Length CV R", ""}},
		AnalogJackOutput4ms{{to_mm<72>(27.2), to_mm<72>(248.15), Center, "Out L", ""}},
		AnalogJackOutput4ms{{to_mm<72>(260.66), to_mm<72>(248.15), Center, "Out R", ""}},
		AnalogJackOutput4ms{{to_mm<72>(68.86), to_mm<72>(283.45), Center, "End Out L", ""}},
		AnalogJackOutput4ms{{to_mm<72>(219.17), to_mm<72>(283.45), Center, "End Out R", ""}},
		AnalogJackInput4ms{{to_mm<72>(106.47), to_mm<72>(244.71), Center, "Left Rec", ""}},
		GateJackInput4ms{{to_mm<72>(143.45), to_mm<72>(263.71), Center, "Rec Jack", ""}},
		AnalogJackInput4ms{{to_mm<72>(180.84), to_mm<72>(244.71), Center, "Right Rec", ""}},
		WhiteLight{{to_mm<72>(14.74), to_mm<72>(227.18), Center, "Play L Light", ""}},
		BlueLight{{to_mm<72>(133.57), to_mm<72>(235.07), Center, "Monitor Light", ""}},
		RedLight{{to_mm<72>(153.69), to_mm<72>(235.07), Center, "Busy Light", ""}},
		WhiteLight{{to_mm<72>(273.54), to_mm<72>(227.18), Center, "Play R Light", ""}},
	}};

	enum class Elem {
		PitchLKnob,
		SampleLKnob,
		StartPos_LKnob,
		LengthLKnob,
		PlayLButton,
		BankLButton,
		ReverseLButton,
		PitchRKnob,
		SampleRKnob,
		StartPos_RKnob,
		LengthRKnob,
		PlayRButton,
		BankRButton,
		ReverseRButton,
		Rec_SampleKnob,
		RecButton,
		Rec_BankButton,
		PlayTrigLIn,
		_1V_OctLIn,
		PlayTrigRIn,
		_1V_OctRIn,
		ReverseTrigLIn,
		ReverseTrigRIn,
		LengthCvLIn,
		StartPosCvLIn,
		SampleCvLIn,
		SampleCvRIn,
		StartPosCvRIn,
		LengthCvRIn,
		OutLOut,
		OutROut,
		EndOutLOut,
		EndOutROut,
		LeftRecIn,
		RecJackIn,
		RightRecIn,
		PlayLLight,
		MonitorLight,
		BusyLight,
		PlayRLight,
	};

	// Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)

	enum {
		KnobPitch_L,
		KnobSample_L,
		KnobStart_Pos__L,
		KnobLength_L,
		KnobPitch_R,
		KnobSample_R,
		KnobStart_Pos__R,
		KnobLength_R,
		KnobRec__Sample,
		NumKnobs,
	};

	enum {
		SwitchPlay_L,
		SwitchBank_L,
		SwitchReverse_L,
		SwitchPlay_R,
		SwitchBank_R,
		SwitchReverse_R,
		SwitchRec,
		SwitchRec__Bank,
		NumSwitches,
	};

	enum {
		InputPlay_Trig_L,
		Input_1V_Oct_L,
		InputPlay_Trig_R,
		Input_1V_Oct_R,
		InputReverse_Trig_L,
		InputReverse_Jack_R,
		InputLength_Cv_L,
		InputStart_Pos_Cv_L,
		InputSample_Cv_L,
		InputSample_Cv_R,
		InputStart_Pos_Cv_R,
		InputLength_Cv_R,
		InputLeft_Rec,
		InputRec_Jack,
		InputRight_Rec,
		NumInJacks,
	};

	enum {
		OutputOut_L,
		OutputOut_R,
		OutputEnd_Out_L,
		OutputEnd_Out_R,
		NumOutJacks,
	};

	enum {
		LedPlay_L_Light,
		LedMonitor_Light,
		LedBusy_Light,
		LedPlay_R_Light,
		NumDiscreteLeds,
	};
};
} // namespace MetaModule
