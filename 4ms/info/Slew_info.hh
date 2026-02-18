#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct SlewInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Slew"};
    static constexpr std::string_view description{"Slew Limiter"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Slew_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Slew.png"};

    using enum Coords;

    static constexpr std::array<Element, 4> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Rise", ""}, 0.5f, 1.0, 2000.0, "ms"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Fall", ""}, 0.5f, 1.0, 2000.0, "ms"},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Signal In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Slewed Out", ""}},
}};

    enum class Elem {
        RiseKnob,
        FallKnob,
        SignalIn,
        SlewedOut,
    };

    // Legacy naming
    
    enum {
        KnobRise, 
        KnobFall, 
        NumKnobs,
    };
    
    
    enum {
        InputSignal_In, 
        NumInJacks,
    };
    
    enum {
        OutputSlewed_Out,
        NumOutJacks,
    };

	// Only used in VCV, not in MM firmware
    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputSignal_In, OutputSlewed_Out}}};

};
} // namespace MetaModule
