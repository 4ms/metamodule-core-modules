#pragma once
#include "helpers/4ms_elements.hh"
#include "CoreModules/elements/element_info.hh"

#include <array>

namespace MetaModule
{
struct TSPInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"TSP"};
    static constexpr std::string_view description{"Sample Player"};
    static constexpr uint32_t width_hp = 6;
    static constexpr std::string_view svg_filename{"res/modules/TSP_artwork.svg"};
    static constexpr std::string_view png_filename{"4ms/fp/TSP.png"};

    using enum Coords;

    static constexpr std::array<Element, 15> Elements{{
		MomentaryRGB7mm{{to_mm<72>(26.196), to_mm<72>(153.596), Center, "Play", ""}},
		OrangeButton{{to_mm<72>(61.467), to_mm<72>(153.596), Center, "Loop", ""}},
		GateJackInput4ms{{to_mm<72>(25.131), to_mm<72>(198.047), Center, "Play Trig", ""}},
		GateJackInput4ms{{to_mm<72>(61.274), to_mm<72>(198.047), Center, "Loop Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(25.131), to_mm<72>(294.539), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(294.822), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(24.684), to_mm<72>(249.486), Center, "End Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(249.486), Center, "Position", ""}},
		DynamicGraphicDisplay{{to_mm<72>(9.234), to_mm<72>(60.753), TopLeft, "Waveform", "", to_mm<72>(67.938), to_mm<72>(66.398)}},
		TSPDisplay{{to_mm<72>(9.056), to_mm<72>(37.165), TopLeft, "Message", "", to_mm<72>(67.938), to_mm<72>(23.588)}},
		RedLight{{to_mm<72>(43.203), to_mm<72>(140.68), Center, "Busy Light", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(66.469), to_mm<72>(48.461), Center, "Play Retrig Mode", ""}, 3, 0}, {"Retrigger", "Stop", "Pause"}},
		AltParamContinuous{{to_mm<72>(51.827), to_mm<72>(48.461), Center, "Waveform Zoom", ""}, 0.1015625f},
		AltParamChoiceLabeled{{{to_mm<72>(36.276), to_mm<72>(48.461), Center, "Buffer Threshold", ""}, 5, 1}, {"Minimal", "20%", "40%", "60%", "80%"}},
		AltParamAction{{to_mm<72>(23.474), to_mm<72>(48.461), Center, "Load Sample...", ""}},
}};

    enum class Elem {
        PlayButton,
        LoopButton,
        PlayTrigIn,
        LoopGateIn,
        LeftOut,
        RightOut,
        EndOut,
        PositionOut,
        WaveformDisplay,
        MessageDisplay,
        BusyLight,
        PlayRetrigModeAltParam,
        WaveformZoomAltParam,
        BufferThresholdAltParam,
        LoadSampleAltParam,
    };

    // Legacy naming
    
    
    enum {
        SwitchPlay, 
        SwitchLoop, 
        NumSwitches,
    };
    
    enum {
        InputPlay_Trig, 
        InputLoop_Gate, 
        NumInJacks,
    };
    
    enum {
        OutputLeft_Out, 
        OutputRight_Out, 
        OutputEnd_Out, 
        OutputPosition, 
        NumOutJacks,
    };
    
    enum {
        LedWaveform, 
        LedMessage, 
        LedBusy_Light, 
        NumDiscreteLeds,
    };
    
    enum {
        AltParamLoad_Sample___, 
        AltParamPlay_Retrig_Mode, 
        AltParamWaveform_Zoom, 
        AltParamBuffer_Threshold, 
    };

};
} // namespace MetaModule
