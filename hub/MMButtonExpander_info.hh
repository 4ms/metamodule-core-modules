#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"

#include <array>

namespace MetaModule
{
struct MMButtonExpanderInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"MMButtonExpander"};
	static constexpr std::string_view description{"MetaModule Button Expander"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view svg_filename{"res/modules/MMButtonExpander_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/MMButtonExpander.png"};

	using enum Coords;

	static constexpr std::array<Element, 8> Elements{{
		BlackMomentary7mm{{to_mm<72>(62.31), to_mm<72>(74.01), Center, "Button1", ""}},
		BlackMomentary7mm{{to_mm<72>(24.87), to_mm<72>(103.29), Center, "Button2", ""}},
		BlackMomentary7mm{{to_mm<72>(62.31), to_mm<72>(136.41), Center, "Button3", ""}},
		BlackMomentary7mm{{to_mm<72>(24.87), to_mm<72>(165.69), Center, "Button4", ""}},
		BlackMomentary7mm{{to_mm<72>(62.31), to_mm<72>(201.21), Center, "Button5", ""}},
		BlackMomentary7mm{{to_mm<72>(24.87), to_mm<72>(230.49), Center, "Button6", ""}},
		BlackMomentary7mm{{to_mm<72>(62.31), to_mm<72>(264.11), Center, "Button7", ""}},
		BlackMomentary7mm{{to_mm<72>(24.87), to_mm<72>(293.37), Center, "Button8", ""}},
	}};

	enum class Elem {
		Button1Button,
		Button2Button,
		Button3Button,
		Button5Button,
		Button4Button,
		Button6Button,
		Button7Button,
		Button8Button,
	};

	// Legacy naming

	enum {
		SwitchButton1,
		SwitchButton2,
		SwitchButton3,
		SwitchButton5,
		SwitchButton4,
		SwitchButton6,
		SwitchButton7,
		SwitchButton8,
		NumSwitches,
	};
};
} // namespace MetaModule
