#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"
#include "helpers/button_utils_elements.hh"

#include <array>

namespace MetaModule
{
struct KeyboardInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"Keyboard"};
	static constexpr std::string_view description{"Keyboard"};
	static constexpr uint32_t width_hp = 20;
	static constexpr std::string_view svg_filename{"res/modules/Keyboard_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/Keyboard.png"};

	using enum Coords;

	// Coordinates are in mm (taken directly from the VCV panel, which used mm2px).
	// Elements are grouped by type so the processor can address key/voice N as
	// (first-of-group + N) at compile time.
	static constexpr std::array<Element, 57> Elements{{
		// 12 momentary key buttons with an RGB LED (VCV: VCVLightBezel<RedGreenBlueLight>)
		MomentaryRGB5mm{{19.034, 30.402, Center, "C", ""}},
		MomentaryRGB5mm{{24.358, 16.687, Center, "C#", ""}},
		MomentaryRGB5mm{{29.682, 30.402, Center, "D", ""}},
		MomentaryRGB5mm{{35.006, 16.687, Center, "D#", ""}},
		MomentaryRGB5mm{{40.330, 30.401, Center, "E", ""}},
		MomentaryRGB5mm{{50.978, 30.401, Center, "F", ""}},
		MomentaryRGB5mm{{56.302, 16.686, Center, "F#", ""}},
		MomentaryRGB5mm{{61.626, 30.401, Center, "G", ""}},
		MomentaryRGB5mm{{66.950, 16.685, Center, "G#", ""}},
		MomentaryRGB5mm{{72.274, 30.400, Center, "A", ""}},
		MomentaryRGB5mm{{77.598, 16.685, Center, "A#", ""}},
		MomentaryRGB5mm{{82.922, 30.400, Center, "B", ""}},

		// 5 tall vertical slide switches (VCV: CKSSVert7), y = 53.2
		SwitchTallVert{{13.689, 53.2, Center, "Button Behavior", ""}, 3, {"Gate", "Trig", "Latch"}, 0},
		SwitchTallVert{{29.675, 53.2, Center, "Octave", ""}, 5, {"-2", "-1", "0", "+1", "+2"}, 2},
		SwitchTallVert{{45.660, 53.2, Center, "Num Voices", ""}, 8, {"1", "2", "3", "4", "5", "6", "7", "8"}, 3},
		SwitchTallVert{{61.645, 53.2, Center, "Note Priority", ""}, 5, {"Newest Note", "Oldest Note", "Highest Note", "Lowest Note", "Ignore New Notes"}, 0},
		SwitchTallVert{{77.631, 53.2, Center, "Voice Allocation", ""}, 3, {"First Available", "Round Robin", "Random"}, 0},

		// 8 mono CV outputs, y = 93.26
		AnalogJackOutput4ms{{12.243, 93.26, Center, "CV 1", ""}},
		AnalogJackOutput4ms{{23.259, 93.26, Center, "CV 2", ""}},
		AnalogJackOutput4ms{{34.276, 93.26, Center, "CV 3", ""}},
		AnalogJackOutput4ms{{45.292, 93.26, Center, "CV 4", ""}},
		AnalogJackOutput4ms{{56.308, 93.26, Center, "CV 5", ""}},
		AnalogJackOutput4ms{{67.324, 93.26, Center, "CV 6", ""}},
		AnalogJackOutput4ms{{78.341, 93.26, Center, "CV 7", ""}},
		AnalogJackOutput4ms{{89.357, 93.26, Center, "CV 8", ""}},

		// 8 mono Gate outputs, y = 102.8
		GateJackOutput4ms{{12.243, 102.8, Center, "Gate 1", ""}},
		GateJackOutput4ms{{23.259, 102.8, Center, "Gate 2", ""}},
		GateJackOutput4ms{{34.276, 102.8, Center, "Gate 3", ""}},
		GateJackOutput4ms{{45.292, 102.8, Center, "Gate 4", ""}},
		GateJackOutput4ms{{56.308, 102.8, Center, "Gate 5", ""}},
		GateJackOutput4ms{{67.324, 102.8, Center, "Gate 6", ""}},
		GateJackOutput4ms{{78.341, 102.8, Center, "Gate 7", ""}},
		GateJackOutput4ms{{89.357, 102.8, Center, "Gate 8", ""}},

		// Sum / poly outputs
		GateJackOutput4ms{{23.259, 117.429, Center, "Gate Sum", ""}},
		GateJackOutput4ms{{34.276, 117.506, Center, "Gate Sum as Trigger", ""}},
		GateJackOutput4ms{{67.324, 117.506, Center, "Gate Poly", ""}},
		AnalogJackOutput4ms{{78.341, 117.506, Center, "CV Poly", ""}},

		// 8 CV activity LEDs (red), y ~ 83.7
		RedLight{{12.243, 83.736, Center, "CV 1", ""}},
		RedLight{{23.259, 83.744, Center, "CV 2", ""}},
		RedLight{{34.276, 83.752, Center, "CV 3", ""}},
		RedLight{{45.292, 83.736, Center, "CV 4", ""}},
		RedLight{{56.308, 83.744, Center, "CV 5", ""}},
		RedLight{{67.324, 83.753, Center, "CV 6", ""}},
		RedLight{{78.341, 83.744, Center, "CV 7", ""}},
		RedLight{{89.357, 83.752, Center, "CV 8", ""}},

		// 12 voice-number labels, x-aligned with each key, in the gap below the
		// lower button row (~30.4 mm) and above the switches (~51 mm).
		VoiceNumberLabel{{19.034, 39.0, Center, "Voice 1", "", 5.0, 4.0}},
		VoiceNumberLabel{{24.358, 39.0, Center, "Voice 2", "", 5.0, 4.0}},
		VoiceNumberLabel{{29.682, 39.0, Center, "Voice 3", "", 5.0, 4.0}},
		VoiceNumberLabel{{35.006, 39.0, Center, "Voice 4", "", 5.0, 4.0}},
		VoiceNumberLabel{{40.330, 39.0, Center, "Voice 5", "", 5.0, 4.0}},
		VoiceNumberLabel{{50.978, 39.0, Center, "Voice 6", "", 5.0, 4.0}},
		VoiceNumberLabel{{56.302, 39.0, Center, "Voice 7", "", 5.0, 4.0}},
		VoiceNumberLabel{{61.626, 39.0, Center, "Voice 8", "", 5.0, 4.0}},
		VoiceNumberLabel{{66.950, 39.0, Center, "Voice 9", "", 5.0, 4.0}},
		VoiceNumberLabel{{72.274, 39.0, Center, "Voice 10", "", 5.0, 4.0}},
		VoiceNumberLabel{{77.598, 39.0, Center, "Voice 11", "", 5.0, 4.0}},
		VoiceNumberLabel{{82.922, 39.0, Center, "Voice 12", "", 5.0, 4.0}},
	}};

	enum class Elem {
		Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9, Key10, Key11, Key12,
		BehaviorSwitch, OctaveSwitch, NumVoicesSwitch, NotePrioritySwitch, VoiceAllocSwitch,
		CvOut1, CvOut2, CvOut3, CvOut4, CvOut5, CvOut6, CvOut7, CvOut8,
		GateOut1, GateOut2, GateOut3, GateOut4, GateOut5, GateOut6, GateOut7, GateOut8,
		GateSum, TrigSum, GatePoly, CvPoly,
		CvLight1, CvLight2, CvLight3, CvLight4, CvLight5, CvLight6, CvLight7, CvLight8,
		VoiceLabel1, VoiceLabel2, VoiceLabel3, VoiceLabel4, VoiceLabel5, VoiceLabel6,
		VoiceLabel7, VoiceLabel8, VoiceLabel9, VoiceLabel10, VoiceLabel11, VoiceLabel12,
	};
};
} // namespace MetaModule
