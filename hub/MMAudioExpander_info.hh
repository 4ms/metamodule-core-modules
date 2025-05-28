#pragma once
#include "CoreModules/elements/element_info.hh"
#include "helpers/4ms_elements.hh"
#include <array>

namespace MetaModule
{
struct MMAudioExpanderInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"MMAudioExpander"};
	static constexpr std::string_view description{"MetaModule Audio Expander MM AIO"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view svg_filename{"res/modules/MMAudioExpander_artwork.svg"};
	static constexpr std::string_view png_filename{"4ms/fp/MMAudioExpander.png"};

	using enum Coords;

	static constexpr std::array<Element, 14> Elements{{
		AnalogJackOutput4ms{{to_mm<72>(18.38), to_mm<72>(52.21), Center, "In7", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.9), to_mm<72>(52.21), Center, "In8", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.38), to_mm<72>(93.61), Center, "In9", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.9), to_mm<72>(93.61), Center, "In10", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.38), to_mm<72>(135.01), Center, "In11", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.9), to_mm<72>(135.01), Center, "In12", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.43), to_mm<72>(187.22), Center, "Out9", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.94), to_mm<72>(187.22), Center, "Out10", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.43), to_mm<72>(228.62), Center, "Out11", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.94), to_mm<72>(228.62), Center, "Out12", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.43), to_mm<72>(270.02), Center, "Out13", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.94), to_mm<72>(270.02), Center, "Out14", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.43), to_mm<72>(311.42), Center, "Out15", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.94), to_mm<72>(311.42), Center, "Out16", ""}},
	}};

	enum class Elem {
		In7Out,
		In8Out,
		In9Out,
		In10Out,
		In11Out,
		In12Out,
		Out9In,
		Out10In,
		Out11In,
		Out12In,
		Out13In,
		Out14In,
		Out15In,
		Out16In,
	};

	// Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)

	enum {
		InputOut9,
		InputOut10,
		InputOut11,
		InputOut12,
		InputOut13,
		InputOut14,
		InputOut15,
		InputOut16,
		NumInJacks,
	};

	enum {
		OutputIn7,
		OutputIn8,
		OutputIn9,
		OutputIn10,
		OutputIn11,
		OutputIn12,
		NumOutJacks,
	};
};
} // namespace MetaModule
