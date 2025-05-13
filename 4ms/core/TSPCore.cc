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

// #define PRINTF_TSP

#ifdef PRINTF_TSP
#define print_tsp printf
#else
#define print_tsp(...)
#endif

namespace MetaModule
{

class TSPCore : public SmartCoreProcessor<TSPInfo> {
	using enum TSPInfo::Elem;

public:
	TSPCore() {
		fs_thread.start([this]() { process_filesystem(); });
	}

	void update() override {
		using enum PlayState;

		switch (play_state) {
			case Buffering:
				if (stream.is_eof() || stream.frames_available() >= PreBufferThreshold) {
					print_tsp("update(): Buffering=>Playing\n");
					play_state = Playing;
				}
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case Playing: {

				if (stream.frames_available() == 0) {
					print_tsp("EOF: stopping\n");
					play_state = Stopped;
				} else {
					auto left_out = stream.pop_sample();
					// print_tsp("%f\n", left_out);
					setOutput<LeftOut>(left_out * 5.f);

					if (stream.is_stereo())
						setOutput<RightOut>(stream.pop_sample() * 5.f);
					else
						setOutput<RightOut>(0);
				}
			} break;

			case Reset:
			case Stopped:
			case LoadSampleInfo:
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;
		};
	}

	void process_filesystem() {
		using enum PlayState;

		switch (play_state) {
			case Stopped:
				// do nothing
				break;

			case LoadSampleInfo:
				print_tsp("fs: LoadSampleInfo\n");
				if (!stream.init(sample_filename)) {
					print_tsp("Could not load sample\n");
				}
				print_tsp("Loaded Sample info: stopped\n");
				play_state = Stopped;
				break;

			case Reset:
				print_tsp("fs seek %u\n", seek_pos.load());
				stream.seek_pos(seek_pos);
				play_state = Buffering;
				break;

			case Buffering:
			case Playing:
				if (stream.frames_available() < PreBufferThreshold) {
					if (!stream.is_eof())
						stream.push_frames_from_file(1024);
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

	void handle_play_button() {
		if (play_button.went_high()) {
			if (play_state == PlayState::Stopped) {
				print_tsp("Play button: => Reset\n");
				play_state = PlayState::Reset;
			}
		}
	}

	void handle_load_button() {
		if (load_button.went_high()) {
			async_open_file(sample_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
				if (path) {
					sample_filename = path;
					play_state = PlayState::LoadSampleInfo;
					print_tsp("Selected a file '%s' => LoadSampleInfo\n", path);
					free(path);
				}
			});
		}
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;
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
		Reset,
	};

	std::atomic<PlayState> play_state{PlayState::Stopped};
	std::atomic<unsigned> seek_pos = 0;

	StaticString<255> sample_filename = "";
	StaticString<255> sample_dir = "";

	EdgeStateDetector play_button;
	EdgeStateDetector load_button;

	static constexpr size_t PreBufferSamples = 1 * 1024 * 1024;
	WavFileStream<PreBufferSamples> stream;

	static constexpr size_t PreBufferThreshold = 256 * 1024;

	float sample_rate = 48000.f;

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
