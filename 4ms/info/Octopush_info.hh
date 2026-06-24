#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"
#include "helpers/button_utils_elements.hh"

#include <array>

namespace MetaModule
{
struct OctopushInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"Octopush"};
	static constexpr std::string_view description{"Octopush"};
	static constexpr uint32_t width_hp = 30;
	static constexpr std::string_view svg_filename{"res/modules/Octopush_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/Octopush.png"};

	using enum Coords;

	// Elements are grouped by control type so that channel N can be addressed as
	// (first-of-group + N) at compile time in the processor.
	static constexpr std::array<Element, 74> Elements{{
		Toggle2pos{{8.0, 34.819, Center, "Ch. 1 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{25.026, 34.819, Center, "Ch. 2 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{42.049, 34.819, Center, "Ch. 3 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{59.05, 34.819, Center, "Ch. 4 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{76.095, 34.819, Center, "Ch. 5 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{93.118, 34.819, Center, "Ch. 6 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{110.14, 34.819, Center, "Ch. 7 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle2pos{{127.163, 34.819, Center, "Ch. 8 Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},

		Toggle3pos{{8.0, 52.438, Center, "Ch. 1 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{25.026, 52.438, Center, "Ch. 2 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{42.049, 52.438, Center, "Ch. 3 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{59.05, 52.438, Center, "Ch. 4 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{76.095, 52.438, Center, "Ch. 5 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{93.118, 52.438, Center, "Ch. 6 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{110.14, 52.438, Center, "Ch. 7 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{127.163, 52.438, Center, "Ch. 8 Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},

		Knob9mm{{8.0, 67.404, Center, "Ch. 1 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{25.026, 67.404, Center, "Ch. 2 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{42.049, 67.404, Center, "Ch. 3 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{59.05, 67.404, Center, "Ch. 4 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{76.095, 67.404, Center, "Ch. 5 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{93.118, 67.404, Center, "Ch. 6 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{110.14, 67.404, Center, "Ch. 7 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		Knob9mm{{127.163, 67.404, Center, "Ch. 8 Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},

		Toggle3pos{{8.0, 101.5, Center, "Ch. 1 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{25.026, 101.5, Center, "Ch. 2 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{42.049, 101.5, Center, "Ch. 3 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{59.05, 101.5, Center, "Ch. 4 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{76.095, 101.5, Center, "Ch. 5 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{93.118, 101.5, Center, "Ch. 6 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{110.14, 101.5, Center, "Ch. 7 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Toggle3pos{{127.163, 101.5, Center, "Ch. 8 Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},

		WhiteMomentary7mm{{8.0, 115.357, Center, "Ch. 1 Button", ""}},
		WhiteMomentary7mm{{25.026, 115.357, Center, "Ch. 2 Button", ""}},
		WhiteMomentary7mm{{42.049, 115.357, Center, "Ch. 3 Button", ""}},
		WhiteMomentary7mm{{59.05, 115.357, Center, "Ch. 4 Button", ""}},
		WhiteMomentary7mm{{76.095, 115.357, Center, "Ch. 5 Button", ""}},
		WhiteMomentary7mm{{93.118, 115.357, Center, "Ch. 6 Button", ""}},
		WhiteMomentary7mm{{110.14, 115.357, Center, "Ch. 7 Button", ""}},
		WhiteMomentary7mm{{127.163, 115.357, Center, "Ch. 8 Button", ""}},

		AnalogJackOutput4ms{{8.0, 16.565, Center, "Ch. 1 Voltage", ""}},
		AnalogJackOutput4ms{{25.026, 16.565, Center, "Ch. 2 Voltage", ""}},
		AnalogJackOutput4ms{{42.049, 16.565, Center, "Ch. 3 Voltage", ""}},
		AnalogJackOutput4ms{{59.05, 16.565, Center, "Ch. 4 Voltage", ""}},
		AnalogJackOutput4ms{{76.095, 16.565, Center, "Ch. 5 Voltage", ""}},
		AnalogJackOutput4ms{{93.118, 16.565, Center, "Ch. 6 Voltage", ""}},
		AnalogJackOutput4ms{{110.14, 16.565, Center, "Ch. 7 Voltage", ""}},
		AnalogJackOutput4ms{{127.163, 16.565, Center, "Ch. 8 Voltage", ""}},

		GateJackOutput4ms{{8.0, 82.034, Center, "Ch. 1 Logic", ""}},
		GateJackOutput4ms{{25.026, 82.034, Center, "Ch. 2 Logic", ""}},
		GateJackOutput4ms{{42.049, 82.034, Center, "Ch. 3 Logic", ""}},
		GateJackOutput4ms{{59.05, 82.034, Center, "Ch. 4 Logic", ""}},
		GateJackOutput4ms{{76.095, 82.034, Center, "Ch. 5 Logic", ""}},
		GateJackOutput4ms{{93.118, 82.034, Center, "Ch. 6 Logic", ""}},
		GateJackOutput4ms{{110.14, 82.034, Center, "Ch. 7 Logic", ""}},
		GateJackOutput4ms{{127.163, 82.034, Center, "Ch. 8 Logic", ""}},

		RedGreenLight{{8.0, 8.485, Center, "Ch. 1", ""}},
		RedGreenLight{{25.026, 8.485, Center, "Ch. 2", ""}},
		RedGreenLight{{42.049, 8.485, Center, "Ch. 3", ""}},
		RedGreenLight{{59.05, 8.485, Center, "Ch. 4", ""}},
		RedGreenLight{{76.095, 8.485, Center, "Ch. 5", ""}},
		RedGreenLight{{93.118, 8.485, Center, "Ch. 6", ""}},
		RedGreenLight{{110.14, 8.485, Center, "Ch. 7", ""}},
		RedGreenLight{{127.163, 8.485, Center, "Ch. 8", ""}},

		Knob9mm{{143.332, 25.923, Center, "Mixer Offset", ""}, 0.5f, -5.0f, 5.0f, "V"},
		RedGreenLight{{143.332, 17.546, Center, "Offset", ""}},
		AnalogJackOutput4ms{{143.332, 46.853, Center, "Mixer Sum", ""}},
		RedGreenLight{{143.332, 40.12, Center, "Sum", ""}},
		AnalogJackOutput4ms{{143.332, 68.786, Center, "Mixer Inverse Sum", ""}},
		RedGreenLight{{143.332, 62.053, Center, "Inverse Sum", ""}},
		AnalogJackOutput4ms{{143.332, 90.718, Center, "Mixer Positive", ""}},
		GreenLight{{143.332, 83.985, Center, "Positive", ""}},
		AnalogJackOutput4ms{{143.332, 112.65, Center, "Mixer Negative", ""}},
		RedLight{{143.332, 105.918, Center, "Negative", ""}},
	}};

	enum class Elem {
		Ch1Polarity, Ch2Polarity, Ch3Polarity, Ch4Polarity, Ch5Polarity, Ch6Polarity, Ch7Polarity, Ch8Polarity,
		Ch1Range, Ch2Range, Ch3Range, Ch4Range, Ch5Range, Ch6Range, Ch7Range, Ch8Range,
		Ch1Amplitude, Ch2Amplitude, Ch3Amplitude, Ch4Amplitude, Ch5Amplitude, Ch6Amplitude, Ch7Amplitude, Ch8Amplitude,
		Ch1Behavior, Ch2Behavior, Ch3Behavior, Ch4Behavior, Ch5Behavior, Ch6Behavior, Ch7Behavior, Ch8Behavior,
		Ch1Push, Ch2Push, Ch3Push, Ch4Push, Ch5Push, Ch6Push, Ch7Push, Ch8Push,
		Ch1VoltageOut, Ch2VoltageOut, Ch3VoltageOut, Ch4VoltageOut, Ch5VoltageOut, Ch6VoltageOut, Ch7VoltageOut, Ch8VoltageOut,
		Ch1LogicOut, Ch2LogicOut, Ch3LogicOut, Ch4LogicOut, Ch5LogicOut, Ch6LogicOut, Ch7LogicOut, Ch8LogicOut,
		Ch1Rgb, Ch2Rgb, Ch3Rgb, Ch4Rgb, Ch5Rgb, Ch6Rgb, Ch7Rgb, Ch8Rgb,
		MixerOffsetKnob,
		OffsetLight,
		SumOut,
		SumLight,
		InvOut,
		InvLight,
		PosOut,
		PosLight,
		NegOut,
		NegLight,
	};
};
} // namespace MetaModule
