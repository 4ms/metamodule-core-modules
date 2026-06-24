#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"
#include "helpers/button_utils_elements.hh"

#include <array>

namespace MetaModule
{
struct SoloPushInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"SoloPush"};
	static constexpr std::string_view description{"Solo Push"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view svg_filename{"res/modules/SoloPush_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/SoloPush.png"};

	using enum Coords;

	static constexpr std::array<Element, 8> Elements{{
		Toggle2pos{{10.16, 35.812, Center, "Polarity", ""}, {"Unipolar", "Bipolar"}, Toggle2pos::State_t::DOWN},
		Toggle3pos{{10.16, 53.431, Center, "Range", ""}, {"1V", "5V", "10V"}, Toggle3pos::State_t::CENTER},
		Toggle3pos{{10.16, 102.5, Center, "Behavior", ""}, {"Gate", "Toggle", "Trig"}, Toggle3pos::State_t::DOWN},
		Knob9mm{{10.16, 68.397, Center, "Amplitude", ""}, 1.0f, 0.0f, 100.0f, "%"},
		WhiteMomentary7mm{{10.16, 116.35, Center, "Button", ""}},
		AnalogJackOutput4ms{{10.16, 17.558, Center, "Voltage", ""}},
		GateJackOutput4ms{{10.16, 83.027, Center, "Logic", ""}},
		RedGreenLight{{10.16, 9.478, Center, "Voltage", ""}},
	}};

	enum class Elem {
		PolaritySwitch,
		RangeSwitch,
		BehaviorSwitch,
		AmplitudeKnob,
		PushButton,
		VoltageOut,
		LogicOut,
		RgbLight,
	};
};
} // namespace MetaModule
