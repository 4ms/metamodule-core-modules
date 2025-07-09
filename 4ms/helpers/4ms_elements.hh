#pragma once
#include "CoreModules/elements/base_element.hh"
#include "util/colors_rgb565.hh"

namespace MetaModule
{

//
// Knobs
//
struct Knob9mm : Knob {
	constexpr Knob9mm() = default;
	constexpr Knob9mm(
		BaseElement b, float defaultValue = 0.5f, float minValue = 0, float maxValue = 1, std::string_view units = "")
		: Knob{{{b, "4ms/comp/knob9mm_x.png"}, defaultValue, minValue, maxValue}} {
		this->units = units;
	}
};

struct DaviesLargeKnob : Knob {
	constexpr DaviesLargeKnob() = default;
	constexpr DaviesLargeKnob(
		BaseElement b, float defaultValue = 0.5f, float minValue = 0, float maxValue = 1, std::string_view units = "")
		: Knob{{{b, "4ms/comp/knob_large_x.png"}, defaultValue, minValue, maxValue}} {
		this->units = units;
	}
};

struct Davies1900hBlackKnob : Knob {
	constexpr Davies1900hBlackKnob() = default;
	constexpr Davies1900hBlackKnob(
		BaseElement b, float defaultValue = 0.5f, float minValue = 0, float maxValue = 1, std::string_view units = "")
		: Knob{{{b, "4ms/comp/knob_x.png"}, defaultValue, minValue, maxValue}} {
		this->units = units;
	}
};

struct DivMultKnob_d32x16 : KnobSnapped {
	constexpr DivMultKnob_d32x16() = default;
	constexpr DivMultKnob_d32x16(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob_x.png"}, defaultValue, 0, 18}}} {
		this->units = units;
		num_pos = 19;
		pos_names[0] = "/32";
		pos_names[1] = "/16";
		pos_names[2] = "/8";
		pos_names[3] = "/7";
		pos_names[4] = "/6";
		pos_names[5] = "/5";
		pos_names[6] = "/4";
		pos_names[7] = "/3";
		pos_names[8] = "/2";
		pos_names[9] = "=";
		pos_names[10] = "x2";
		pos_names[11] = "x3";
		pos_names[12] = "x4";
		pos_names[13] = "x5";
		pos_names[14] = "x6";
		pos_names[15] = "x7";
		pos_names[16] = "x8";
		pos_names[17] = "x12";
		pos_names[18] = "x16";
	}
};

struct DivMultKnob_d8x8 : KnobSnapped {
	constexpr DivMultKnob_d8x8() = default;
	constexpr DivMultKnob_d8x8(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob_x.png"}, defaultValue, 0, 14}}} {
		this->units = units;
		num_pos = 15;
		pos_names[0] = "/8";
		pos_names[1] = "/7";
		pos_names[2] = "/6";
		pos_names[3] = "/5";
		pos_names[4] = "/4";
		pos_names[5] = "/3";
		pos_names[6] = "/2";
		pos_names[7] = "=";
		pos_names[8] = "x2";
		pos_names[9] = "x3";
		pos_names[10] = "x4";
		pos_names[11] = "x5";
		pos_names[12] = "x6";
		pos_names[13] = "x7";
		pos_names[14] = "x8";
		min_angle = -142;
		max_angle = 142;
	}
};

struct DivMultKnobDLD : KnobSnapped {
	constexpr DivMultKnobDLD() = default;
	constexpr DivMultKnobDLD(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob_x.png"}, defaultValue, 0, 16}}} {
		this->units = units;
		num_pos = 17;
		pos_names[0] = "x1";
		pos_names[1] = "x1.5";
		pos_names[2] = "x2";
		pos_names[3] = "x3";
		pos_names[4] = "x4";
		pos_names[5] = "x5";
		pos_names[6] = "x6";
		pos_names[7] = "x7";
		pos_names[8] = "x8";
		pos_names[9] = "x9";
		pos_names[10] = "x10";
		pos_names[11] = "x11";
		pos_names[12] = "x12";
		pos_names[13] = "x13";
		pos_names[14] = "x14";
		pos_names[15] = "x15";
		pos_names[16] = "x16";
		min_angle = -145;
		max_angle = 145;
	}
};

struct DivMultKnob_d1d16 : KnobSnapped {
	constexpr DivMultKnob_d1d16() = default;
	constexpr DivMultKnob_d1d16(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob9mm_x.png"}, defaultValue, 0, 15}}} {
		this->units = units;
		num_pos = 16;
		pos_names[0] = "=";
		pos_names[1] = "/2";
		pos_names[2] = "/3";
		pos_names[3] = "/4";
		pos_names[4] = "/5";
		pos_names[5] = "/6";
		pos_names[6] = "/7";
		pos_names[7] = "/8";
		pos_names[8] = "/9";
		pos_names[9] = "/10";
		pos_names[10] = "/11";
		pos_names[11] = "/12";
		pos_names[12] = "/13";
		pos_names[13] = "/14";
		pos_names[14] = "/15";
		pos_names[15] = "/16";
	}
};

struct DivMultKnob_x1x16 : KnobSnapped {
	constexpr DivMultKnob_x1x16() = default;
	constexpr DivMultKnob_x1x16(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob9mm_x.png"}, defaultValue, 0, 15}}} {
		this->units = units;
		num_pos = 16;
		pos_names[0] = "x1";
		pos_names[1] = "x2";
		pos_names[2] = "x3";
		pos_names[3] = "x4";
		pos_names[4] = "x5";
		pos_names[5] = "x6";
		pos_names[6] = "x7";
		pos_names[7] = "x8";
		pos_names[8] = "x9";
		pos_names[9] = "x10";
		pos_names[10] = "x11";
		pos_names[11] = "x12";
		pos_names[12] = "x13";
		pos_names[13] = "x14";
		pos_names[14] = "x15";
		pos_names[15] = "x16";
	}
};

struct OctaveKnob : KnobSnapped {
	constexpr OctaveKnob() = default;
	constexpr OctaveKnob(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob9mm_x.png"}, defaultValue, 0, 8}}} {
		this->units = units;
		num_pos = 9;
		pos_names[0] = "-4";
		pos_names[1] = "-3";
		pos_names[2] = "-2";
		pos_names[3] = "-1";
		pos_names[4] = "0";
		pos_names[5] = "+1";
		pos_names[6] = "+2";
		pos_names[7] = "+3";
		pos_names[8] = "+4";
		min_angle = -135;
		max_angle = 135;
	}
};

struct Knob_1_10 : KnobSnapped {
	constexpr Knob_1_10() = default;
	constexpr Knob_1_10(BaseElement b, float defaultValue = 0.5f)
		: KnobSnapped{{{{b, "4ms/comp/knob_x.png"}, defaultValue, 0, 9}}} {
		this->units = units;
		num_pos = 10;
		pos_names[0] = "1";
		pos_names[1] = "2";
		pos_names[2] = "3";
		pos_names[3] = "4";
		pos_names[4] = "5";
		pos_names[5] = "6";
		pos_names[6] = "7";
		pos_names[7] = "8";
		pos_names[8] = "9";
		pos_names[9] = "10";
	}
};

//
// Sliders
//
struct Slider25mmHorizLED : SliderLight {
	constexpr Slider25mmHorizLED() = default;
	constexpr Slider25mmHorizLED(
		BaseElement b, float defaultValue = 0.5f, float minValue = 0, float maxValue = 1, std::string_view units = "")
		: SliderLight{{{{b, "4ms/comp/slider_horiz_x.png"}, defaultValue, minValue, maxValue},
					   "4ms/comp/slider_horiz_handle_x.png"}} {
		this->units = units;
	}
};

struct Slider25mmVertLED : SliderLight {
	constexpr Slider25mmVertLED() = default;
	constexpr Slider25mmVertLED(
		BaseElement b, float defaultValue = 0.5f, float minValue = 0, float maxValue = 1, std::string_view units = "")
		: SliderLight{
			  {{{b, "4ms/comp/slider_x.png"}, defaultValue, minValue, maxValue}, "4ms/comp/slider_handle_x.png"}} {
		this->units = units;
	}
};

//
// Buttons
//
struct OrangeButton : LatchingButton {
	constexpr OrangeButton(BaseElement b, State_t defaultValue = State_t::UP)
		: LatchingButton{{b, "4ms/comp/button_x.png"}, defaultValue, Colors565::Orange} {
	}
};

struct WhiteMomentary7mm : MomentaryButtonLight {
	constexpr WhiteMomentary7mm() = default;
	constexpr WhiteMomentary7mm(BaseElement b)
		: MomentaryButtonLight{{{b, "4ms/comp/button_x.png"}, ""}, Colors565::White} {
	}
};

struct MomentaryRGB7mm : MomentaryButtonRGB {
	constexpr MomentaryRGB7mm() = default;
	constexpr MomentaryRGB7mm(BaseElement b)
		: MomentaryButtonRGB{{{b, "4ms/comp/button_x.png"}, ""}} {
	}
};

struct MomentaryRGB5mm : MomentaryButtonRGB {
	constexpr MomentaryRGB5mm() = default;
	constexpr MomentaryRGB5mm(BaseElement b)
		: MomentaryButtonRGB{{{b, "4ms/comp/button_x.png"}, ""}} {
	}
};

//
// Switches
//
struct Toggle2pos : FlipSwitch {
	enum State_t : FlipSwitch::State_t { DOWN = 0, UP = 1 };

	constexpr Toggle2pos() = default;
	constexpr Toggle2pos(BaseElement b,
						 std::array<std::string_view, 2> names = {"Down", "Up"},
						 State_t defaultValue = State_t::DOWN)
		: FlipSwitch{
			  {{b}, 2, defaultValue},
			  {"4ms/comp/switch_down.png", "4ms/comp/switch_up.png"},
			  {names[0], names[1]},
		  } {
	}
};

struct Toggle3pos : FlipSwitch {
	enum State_t : FlipSwitch::State_t { DOWN = 0, CENTER = 1, UP = 2 };

	constexpr Toggle3pos() = default;
	constexpr Toggle3pos(BaseElement b,
						 std::array<std::string_view, 3> names = {"Down", "Center", "Up"},
						 State_t defaultValue = State_t::DOWN)
		: FlipSwitch{
			  {{b}, 3, defaultValue},
			  {"4ms/comp/switch_down.png", "4ms/comp/switch_center.png", "4ms/comp/switch_up.png"},
			  {names[0], names[1], names[2]},
		  } {
	}
};

struct Toggle2posHoriz : FlipSwitch {
	enum State_t : FlipSwitch::State_t { LEFT = 0, RIGHT = 1 };

	constexpr Toggle2posHoriz() = default;
	constexpr Toggle2posHoriz(BaseElement b,
							  std::array<std::string_view, 2> names = {"Left", "Right"},
							  State_t defaultValue = State_t::LEFT)
		: FlipSwitch{
			  {{b}, 2, defaultValue},
			  {"4ms/comp/switch_horiz_left.png", "4ms/comp/switch_horiz_right.png"},
			  {names[0], names[1]},
		  } {
	}
};

struct Toggle3posHoriz : FlipSwitch {
	enum State_t : FlipSwitch::State_t { LEFT = 0, CENTER = 1, RIGHT = 2 };

	constexpr Toggle3posHoriz() = default;
	constexpr Toggle3posHoriz(BaseElement b,
							  std::array<std::string_view, 3> names = {"Left", "Center", "Right"},
							  State_t defaultValue = State_t::LEFT)
		: FlipSwitch{
			  {{b}, 3, defaultValue},
			  {"4ms/comp/switch_horiz_left.png", "4ms/comp/switch_horiz_center.png", "4ms/comp/switch_horiz_right.png"},
			  {names[0], names[1], names[2]},
		  } {
	}
};

struct Encoder9mmRGB : EncoderRGB {
	constexpr Encoder9mmRGB(BaseElement b)
		: EncoderRGB{b, "4ms/comp/knob_unlined_x.png"} {
	}
};

//
// Input Jacks
//

struct GateJackInput4ms : JackInput {
	constexpr GateJackInput4ms(BaseElement b)
		: JackInput{b, "4ms/comp/jack_x.png"} {
	}
};
struct AnalogJackInput4ms : JackInput {
	constexpr AnalogJackInput4ms(BaseElement b)
		: JackInput{b, "4ms/comp/jack_x.png"} {
	}
};

//
// Output jacks
//

struct GateJackOutput4ms : JackOutput {
	constexpr GateJackOutput4ms(BaseElement b)
		: JackOutput{b, "4ms/comp/jack_x.png"} {
	}
};
struct AnalogJackOutput4ms : JackOutput {
	constexpr AnalogJackOutput4ms(BaseElement b)
		: JackOutput{b, "4ms/comp/jack_x.png"} {
	}
};

//
// Lights
//

struct RedLight : MonoLight {
	constexpr RedLight(BaseElement b)
		: MonoLight{{b, "4ms/comp/led_x.png"}, Colors565::Red} {
	}
};

struct BlueLight : MonoLight {
	constexpr BlueLight(BaseElement b)
		: MonoLight{{b, "4ms/comp/led_x.png"}, Colors565::Blue} {
	}
};

struct WhiteLight : MonoLight {
	constexpr WhiteLight(BaseElement b)
		: MonoLight{{b, "4ms/comp/led_x.png"}, Colors565::White} {
	}
};

struct GreenLight : MonoLight {
	constexpr GreenLight(BaseElement b)
		: MonoLight{{b, "4ms/comp/led_x.png"}, Colors565::Green} {
	}
};

struct OrangeLight : MonoLight {
	constexpr OrangeLight(BaseElement b)
		: MonoLight{{b, "4ms/comp/led_x.png"}, Colors565::Orange} {
	}
};

struct RedBlueLight : DualLight {
	constexpr RedBlueLight(BaseElement b)
		: DualLight{{b, "4ms/comp/led_x.png"}, {Colors565::Red, Colors565::Blue}} {
	}
};

struct RedGreenBlueLight : RgbLight {
	constexpr RedGreenBlueLight(BaseElement b)
		: RgbLight{{b, "4ms/comp/led_x.png"}} {
	}
};

struct TSPDisplay : DynamicTextDisplay {
	constexpr TSPDisplay(BaseElement b)
		: DynamicTextDisplay{{{b}}} {
		text = "Load a sample";
		font = "Default_12";
		color = Colors565::White;
		wrap_mode = WrapMode::Scroll;
	}
};

struct GraphicDisplay : DynamicGraphicDisplay {
	constexpr GraphicDisplay(BaseElement b)
		: DynamicGraphicDisplay{{{b}}} {
	}
};

} // namespace MetaModule
