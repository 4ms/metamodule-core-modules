#pragma once
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct MMAIOInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"MMAudioExpander"};
    static constexpr std::string_view description{"MetaModule Audio Expander (MM AIO)"};
    static constexpr uint32_t width_hp = 6;
    static constexpr std::string_view svg_filename{"res/modules/MMAIO_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/MMAIO.png"};

    using enum Coords;

    static constexpr std::array<Element, 14> Elements{{
		AnalogJackInput4ms{{to_mm<72>(18.38), to_mm<72>(52.21), Center, "In7", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.9), to_mm<72>(52.21), Center, "In8", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.38), to_mm<72>(93.61), Center, "In9", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.9), to_mm<72>(93.61), Center, "In10", ""}},
		AnalogJackInput4ms{{to_mm<72>(18.38), to_mm<72>(135.01), Center, "In11", ""}},
		AnalogJackInput4ms{{to_mm<72>(61.9), to_mm<72>(135.01), Center, "In12", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.43), to_mm<72>(187.22), Center, "Out9", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.94), to_mm<72>(187.22), Center, "Out10", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.43), to_mm<72>(228.62), Center, "Out11", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.94), to_mm<72>(228.62), Center, "Out12", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.43), to_mm<72>(270.02), Center, "Out13", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.94), to_mm<72>(270.02), Center, "Out14", ""}},
		AnalogJackOutput4ms{{to_mm<72>(18.43), to_mm<72>(311.42), Center, "Out15", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.94), to_mm<72>(311.42), Center, "Out16", ""}},
}};

    enum class Elem {
        In7In,
        In8In,
        In9In,
        In10In,
        In11In,
        In12In,
        Out9Out,
        Out10Out,
        Out11Out,
        Out12Out,
        Out13Out,
        Out14Out,
        Out15Out,
        Out16Out,
    };

    // Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)
    
    
    
    enum {
        InputIn7, 
        InputIn8, 
        InputIn9, 
        InputIn10, 
        InputIn11, 
        InputIn12, 
        NumInJacks,
    };
    
    enum {
        OutputOut9, 
        OutputOut10, 
        OutputOut11, 
        OutputOut12, 
        OutputOut13, 
        OutputOut14, 
        OutputOut15, 
        OutputOut16, 
        NumOutJacks,
    };
    
    
};
} // namespace MetaModule
