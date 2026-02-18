#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct DetuneInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"Detune"};
    static constexpr std::string_view description{"Detuner"};
    static constexpr uint32_t width_hp = 8;
    static constexpr std::string_view svg_filename{"res/modules/Detune_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/Detune.png"};

    using enum Coords;

    static constexpr std::array<Element, 8> Elements{{
		Davies1900hBlackKnob{{to_mm<72>(31.82), to_mm<72>(57.84), Center, "Wow Speed", ""}, 0.25f, 0.1, 5.33, "hz"},
		Davies1900hBlackKnob{{to_mm<72>(83.35), to_mm<72>(57.84), Center, "Flutter Speed", ""}, 0.0f, 5.0, 30.0, "hz"},
		Davies1900hBlackKnob{{to_mm<72>(31.82), to_mm<72>(119.08), Center, "Wow Depth", ""}, 0.5f},
		Davies1900hBlackKnob{{to_mm<72>(83.35), to_mm<72>(119.08), Center, "Flutter Depth", ""}, 0.5f},
		AnalogJackInput4ms{{to_mm<72>(31.99), to_mm<72>(271.96), Center, "Audio In", ""}},
		AnalogJackInput4ms{{to_mm<72>(83.18), to_mm<72>(271.96), Center, "Detune CV In", ""}},
		AnalogJackInput4ms{{to_mm<72>(31.99), to_mm<72>(313.57), Center, "Depth CV In", ""}},
		AnalogJackOutput4ms{{to_mm<72>(83.18), to_mm<72>(313.57), Center, "Audio Out", ""}},
}};

    enum class Elem {
        WowSpeedKnob,
        FlutterSpeedKnob,
        WowDepthKnob,
        FlutterDepthKnob,
        AudioIn,
        DetuneCvIn,
        DepthCvIn,
        AudioOut,
    };

    // Legacy naming
    
    enum {
        KnobWow_Speed, 
        KnobFlutter_Speed, 
        KnobWow_Depth, 
        KnobFlutter_Depth, 
        NumKnobs,
    };
    
    
    enum {
        InputAudio_In, 
        InputDetune_Cv_In, 
        InputDepth_Cv_In, 
        NumInJacks,
    };
    
    enum {
        OutputAudio_Out,
        NumOutJacks,
    };

    // Only used in VCV, not in MM firmware
    static constexpr std::array<BypassRoute, 1> bypass_routes{{{InputAudio_In, OutputAudio_Out}}};

};
} // namespace MetaModule
