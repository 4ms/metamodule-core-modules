#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "filesystem/async_filebrowser.hh"
#include "info/TSP_info.hh"
#include "tsp/wav_file_stream.hh"
#include "util/edge_detector.hh"
#include "util/static_string.hh"
#include <atomic>

//#include "../../../../../src/medium/debug_raw.h"

namespace MetaModule
{

class TSPCore : public SmartCoreProcessor<TSPInfo> {
	using enum TSPInfo::Elem;

public:
	TSPCore() {
		fs_thread.start([this]() { handle_filesystem(); });
	}

	void update() override {
		using enum PlayState;

		switch (play_state) {
			case Buffering:
				if (wav.is_eof() || wav.frames_available() >= PreBufferThreshold) {
					printf("update(): Buffering=>Playing\n");
					play_state = Playing;
				}
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case Playing: {

				if (wav.is_empty()) {
					printf("EOF: stopping\n");
					play_state = Stopped;
				} else {
					auto left_out = wav.get_sample();
					printf("%f\n", left_out);
					setOutput<LeftOut>(left_out * 5.f);

					if (wav.is_stereo())
						setOutput<RightOut>(wav.get_sample() * 5.f);
					else
						setOutput<RightOut>(0);
				}
			} break;

			case Stopped:
			case LoadSampleInfo:
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;
		};
	}

	void handle_filesystem() {
		using enum PlayState;

		switch (play_state) {
			case Stopped:
				// do nothing
				break;

			case LoadSampleInfo:
				printf("fs: LoadSampleInfo\n");
				if (!wav.init(sample_filename)) {
					printf("Could not load sample\n");
				}
				printf("Loaded Sample info: stopped\n");
				play_state = Stopped;
				break;

			case Buffering:
			case Playing:
				if (wav.frames_available() < PreBufferThreshold) {
					if (!wav.is_eof())
						wav.buffer_frames(1024);
				}
				break;
		};
	}

	void set_param(int id, float val) override {
		if (id == param_idx<LoadsampleAltParam>) {
			load_button.update(val > 0.5f);
			handle_load_button();

		} else if (id == param_idx<PlayButton>) {
			play_button.update(val > 0.5f);
			handle_play_button();

		} else {
			SmartCoreProcessor::set_param(id, val);
		}
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;
	}

	void handle_play_button() {
		if (play_button.went_high()) {
			if (play_state == PlayState::Stopped) {
				printf("Play button: => Buffering\n");
				play_state = PlayState::Buffering;
			}
		}
	}

	void handle_load_button() {
		if (load_button.went_high()) {
			async_open_file(sample_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
				if (path) {
					sample_filename = path;
					play_state = PlayState::LoadSampleInfo;
					printf("Selected a file '%s' => LoadSampleInfo\n", path);
					free(path);
				}
			});
		}
	}

	// This runs in the low-pri thread:
	AsyncThread fs_thread{this};

	enum class PlayState {
		Stopped,
		LoadSampleInfo,
		Buffering,
		// FadingUp,
		Playing,
		// FadingDownToStop,
		// FadingDownToLoop,
	};

	std::atomic<PlayState> play_state{PlayState::Stopped};

	StaticString<255> sample_filename = "";
	StaticString<255> sample_dir = "";

	EdgeStateDetector play_button;
	EdgeStateDetector load_button;

	static constexpr size_t PreBufferSamples = 1 * 1024 * 1024;
	WavFileStream<PreBufferSamples> wav;

	static constexpr size_t PreBufferThreshold = 256 * 1024;

	float sample_rate = 48000.f;

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
