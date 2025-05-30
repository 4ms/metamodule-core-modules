#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct FollowInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Follow"};
    static constexpr std::string_view description{"Follower"};
    static constexpr uint32_t width_hp = 4;
    static constexpr std::string_view svg_filename{"res/modules/Follow_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Follow.png"};

    using enum Coords;

    static constexpr std::array<Element, 6> Elements{{
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(46.77), Center, "Rise", ""}, 0.0f, 1.0, 2000.0, "ms"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(94.96), Center, "Fall", ""}, 0.0f, 1.0, 2000.0, "ms"},
		Knob9mm{{to_mm<72>(28.8), to_mm<72>(143.15), Center, "Threshold", ""}, 0.0f, 0.0, 8.0, "v"},
		AnalogJackInput4ms{{to_mm<72>(28.8), to_mm<72>(216.85), Center, "Signal In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(265.04), Center, "Gate Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(28.8), to_mm<72>(313.23), Center, "Envelope Out", ""}},
}};

    enum class Elem {
        RiseKnob,
        FallKnob,
        ThresholdKnob,
        SignalIn,
        GateOut,
        EnvelopeOut,
    };

    // Legacy naming
    
    enum {
        KnobRise, 
        KnobFall, 
        KnobThreshold, 
        NumKnobs,
    };
    
    
    enum {
        InputSignal_In, 
        NumInJacks,
    };
    
    enum {
        OutputGate_Out, 
        OutputEnvelope_Out, 
        NumOutJacks,
    };
    
    

};
} // namespace MetaModule
