#pragma once
#include "CoreModules/elements/base_element.hh"
#include "util/colors_rgb565.hh"

namespace MetaModule
{

// Two-color (red/green) LED, driven with a single BipolarColor_t:
//   positive value -> green, negative value -> red.
struct RedGreenLight : DualLight {
	constexpr RedGreenLight(BaseElement b)
		: DualLight{{b, "4ms/comp/led_x.png"}, {Colors565::Red, Colors565::Green}} {
	}
};

// Tall vertical slide switch with a configurable number of positions.
struct SwitchTallVert : SlideSwitch {
	constexpr SwitchTallVert() = default;
	constexpr SwitchTallVert(BaseElement b,
							 unsigned num_positions,
							 std::array<std::string_view, 8> names = {},
							 unsigned default_pos = 0)
		: SlideSwitch{{{b, "4ms/comp/switch_tall_vert_x.png"}, num_positions, default_pos},
					  "4ms/comp/switch_tall_vert_handle.png",
					  SlideSwitch::Ascend::UpLeft,
					  names} {
	}
};

// Small dynamic text label (one short string), used by Keyboard to show the
// voice number currently assigned to each key.
struct VoiceNumberLabel : DynamicTextDisplay {
	constexpr VoiceNumberLabel(BaseElement b)
		: DynamicTextDisplay{{{b}}} {
		text = "";
		font = "Default_12";
		color = Colors565::White;
		wrap_mode = WrapMode::Clip;
	}
};

} // namespace MetaModule
