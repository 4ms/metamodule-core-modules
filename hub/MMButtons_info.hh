#pragma once
#include "CoreModules/4ms/4ms_elements.hh"
#include "CoreModules/4ms/4ms_element_state_conversions.hh"
#include "CoreModules/elements/element_info.hh"
#include <array>

namespace MetaModule
{
struct MMButtonsInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"MMButtons"};
    static constexpr std::string_view description{"MetaModule Button Expander"};
    static constexpr uint32_t width_hp = 6;
    static constexpr std::string_view svg_filename{"res/modules/MMButtons_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/MMButtons.png"};

    using enum Coords;

    static constexpr std::array<Element, 8> Elements{{
		WhiteMomentary7mm{{to_mm<72>(62.31), to_mm<72>(74.01), Center, "Button1", ""}},
		WhiteMomentary7mm{{to_mm<72>(24.87), to_mm<72>(103.29), Center, "Button2", ""}},
		WhiteMomentary7mm{{to_mm<72>(62.31), to_mm<72>(136.41), Center, "Button3", ""}},
		WhiteMomentary7mm{{to_mm<72>(62.31), to_mm<72>(201.21), Center, "Button5", ""}},
		WhiteMomentary7mm{{to_mm<72>(24.87), to_mm<72>(165.69), Center, "Button4", ""}},
		WhiteMomentary7mm{{to_mm<72>(24.87), to_mm<72>(230.49), Center, "Button6", ""}},
		WhiteMomentary7mm{{to_mm<72>(62.31), to_mm<72>(264.11), Center, "Button7", ""}},
		WhiteMomentary7mm{{to_mm<72>(24.87), to_mm<72>(293.37), Center, "Button8", ""}},
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

    // Legacy naming (safe to remove once all legacy 4ms CoreModules are converted)
    
    
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
