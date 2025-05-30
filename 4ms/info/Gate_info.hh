#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct GateInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Gate"};
    static constexpr std::string_view description{"Gate Delay"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Gate_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Gate.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.85), Center, "Length", ""}, 0.25f, 1.0, 1000.0, "ms"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.99), Center, "Delay", ""}, 0.0f, 0.0, 1000.0, "ms"},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(168.72), Center, "Length CV", ""}},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.89), Center, "Delay CV", ""}},
		GateJackInput4ms{{to_mm<72>(28.8), to_mm<72>(265.06), Center, "Gate In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Gate Out", ""}},
}};

    enum class Elem {
        LengthKnob,
        DelayKnob,
        LengthCvIn,
        DelayCvIn,
        GateIn,
        GateOut,
    };

    // Legacy naming
    
    enum {
        KnobLength, 
        KnobDelay, 
        NumKnobs,
    };
    
    
    enum {
        InputLength_Cv, 
        InputDelay_Cv, 
        InputGate_In, 
        NumInJacks,
    };
    
    enum {
        OutputGate_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
