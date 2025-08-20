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

    static constexpr std::array<Element, 18> Elements{{
		MomentaryRGB7mm{{to_mm<72>(26.196), to_mm<72>(153.596), Center, "Play", ""}},
		OrangeButton{{to_mm<72>(61.467), to_mm<72>(153.596), Center, "Loop", ""}},
		GateJackInput4ms{{to_mm<72>(25.131), to_mm<72>(198.047), Center, "Play Trig", ""}},
		GateJackInput4ms{{to_mm<72>(61.274), to_mm<72>(198.047), Center, "Loop Gate", ""}},
		AnalogJackOutput4ms{{to_mm<72>(25.131), to_mm<72>(294.539), Center, "Left Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(294.822), Center, "Right Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(24.684), to_mm<72>(249.486), Center, "End Out", ""}},
		AnalogJackOutput4ms{{to_mm<72>(61.274), to_mm<72>(249.486), Center, "Position", ""}},
		GraphicDisplay{{to_mm<72>(9.234), to_mm<72>(60.753), TopLeft, "Waveform", "", to_mm<72>(67.938), to_mm<72>(66.398)}},
		TSPDisplay{{to_mm<72>(9.056), to_mm<72>(37.165), TopLeft, "Message", "", to_mm<72>(67.938), to_mm<72>(23.588)}},
		RedLight{{to_mm<72>(43.203), to_mm<72>(140.68), Center, "Busy Light", ""}},
		AltParamChoiceLabeled{{{to_mm<72>(66.469), to_mm<72>(48.461), Center, "Play Retrig Mode", ""}, 3, 0}, {"Retrigger", "Stop", "Pause"}},
		AltParamContinuous{{to_mm<72>(51.827), to_mm<72>(48.461), Center, "Waveform Zoom", ""}, 0.1015625f},
		AltParamContinuous{{to_mm<72>(36.276), to_mm<72>(48.461), Center, "Playback Buffer Threshold", ""}, 0.25f},
		AltParamChoiceLabeled{{{to_mm<72>(30.276), to_mm<72>(48.461), Center, "Max Buffer Size", ""}, 12, 3}, {"1MB (5s 48kHz)", "2MB (10s 48kHz)", "4MB (21s 48kHz)", "8MB (43s 48kHz)", "16MB (1m27s 48kHz)", "24MB (2m11s 48kHz)", "32MB (2m54s 48kHz)", "48MB (4m22s 48kHz)", "64MB (5m49s 48kHz)", "80MB (7m16s 48kHz)", "96MB (8m44s 48kHz)", "128MB (11m38s 48kHz)"}},
		AltParamChoiceLabeled{{{to_mm<72>(30.276), to_mm<72>(48.461), Center, "Buffer Strategy", ""}, 2, 0}, {"Fill to threshold", "Fill completely"}},
		AltParamChoiceLabeled{{{to_mm<72>(30.276), to_mm<72>(48.461), Center, "Startup Delay (sec)", ""}, 13, 0}, {"0", "1", "2", "3", "4", "5", "8", "10", "12", "15", "20", "25", "30"}},
		WavFileBrowseAction{{to_mm<72>(23.474), to_mm<72>(48.461), Center, "Load Sample...", ""}},
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
        PlaybackBufferThresholdAltParam,
        MaxBufferSizeAltParam,
        BufferStrategyAltParam,
        StartupDelay_Sec_AltParam,
        LoadSampleAltParam,
    };

};
} // namespace MetaModule
