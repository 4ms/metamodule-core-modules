#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"

#include <array>

namespace MetaModule
{
struct FadeInInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"FadeIn"};
	static constexpr std::string_view description{"Fade In"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view svg_filename{"res/modules/FadeIn_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/FadeIn.png"};

	using enum Coords;

	// Coordinates are in mm (taken directly from the VCV panel, which used mm2px).
	static constexpr std::array<Element, 12> Elements{{
		// Momentary buttons with a white LED (VCV: LEDBezel + LEDBezelLight<WhiteLight>)
		WhiteMomentary7mm{{8.536, 11.528, Center, "Fade In", ""}},
		WhiteMomentary7mm{{21.892, 11.528, Center, "Reset", ""}},

		// Knobs. Display range/units only; the processor reads the raw 0..1 value.
		// Fade Time default 2s -> normalized (2-0.5)/(10-0.5) = 0.1579
		Davies1900hBlackKnob{{15.24, 34.941, Center, "Fade Time", ""}, 0.15789f, 0.5f, 10.0f, "s"},
		// Shape default 0% -> normalized 0.5
		Davies1900hBlackKnob{{15.24, 61.001, Center, "Shape", ""}, 0.5f, -100.0f, 100.0f, "%"},

		// Jacks
		AnalogJackInput4ms{{8.536, 93.527, Center, "Audio Left", ""}},
		AnalogJackInput4ms{{21.892, 93.527, Center, "Audio Right", ""}},
		AnalogJackOutput4ms{{8.536, 115.05, Center, "Audio Left", ""}},
		AnalogJackOutput4ms{{21.892, 115.05, Center, "Audio Right", ""}},

		// Input/Output level LEDs
		RedLight{{8.536, 85.954, Center, "In Left", ""}},
		RedLight{{21.892, 85.932, Center, "In Right", ""}},
		GreenLight{{8.693, 107.35, Center, "Out Left", ""}},
		GreenLight{{22.049, 107.328, Center, "Out Right", ""}},
	}};

	enum class Elem {
		FadeButton,
		ResetButton,
		FadeTimeKnob,
		ShapeKnob,
		AudioInL,
		AudioInR,
		AudioOutL,
		AudioOutR,
		InLLight,
		InRLight,
		OutLLight,
		OutRLight,
	};
};
} // namespace MetaModule
