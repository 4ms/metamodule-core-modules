#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "filesystem/async_filebrowser.hh"
#include "info/TSP_info.hh"
#include "tsp/wav_file_stream.hh"
#include "util/edge_detector.hh"
#include "util/schmitt_trigger.hh"
#include "util/static_string.hh"
#include <atomic>

static constexpr bool DEBUG_ENDOUT_IS_PREBUFF_AMT = true;

#define print_tsp printf
// #define print_tsp(...)

namespace MetaModule
{

class TSPCore : public SmartCoreProcessor<TSPInfo> {
	using enum TSPInfo::Elem;

public:
	TSPCore() {
		fs_thread.start([this]() { async_process_filesystem(); });
	}

	~TSPCore() {
		fs_thread.stop();
		stream.unload();
	}

	void update() override {
		handle_load_button();
		handle_play();
		handle_loop_toggle();

		using enum PlayState;

		switch (play_state) {
			case Buffering:
				setLED<PlayButton>(0, 0, 1);
				if (stream.is_eof() || stream.frames_available() >= PreBufferThreshold) {
					play_state = Playing;
				}
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case Playing:
				setLED<PlayButton>(0, 1, 0);

				if (stream.frames_available()) {
					if (stream.is_stereo()) {
						setOutput<LeftOut>(stream.pop_sample() * 5.f);
						setOutput<RightOut>(stream.pop_sample() * 5.f);
					} else {
						setOutput<LeftOut>(stream.pop_sample() * 5.f);
						setOutput<RightOut>(0);
					}

					setOutput<EndOut>(0.f);
				} else {
					// No frames avaiable.
					// If we're also at the end of file, then stop.
					// Otherwise, we have a buffer underflow, so just wait until buffer fills up
					if (stream.is_eof()) {
						setOutput<EndOut>(5.f);
						if (loop_mode)
							play_state = Reset;
						else
							play_state = Stopped;
					} else {
						print_tsp("Buffer underflow module %u\n", (unsigned)id);
					}
				}
				break;

			case Reset:
			case Stopped:
				setLED<PlayButton>(0, 0, 0);
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case LoadSampleInfo:
				setLED<PlayButton>(0, 0, 1);
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;
		}

		if constexpr (DEBUG_ENDOUT_IS_PREBUFF_AMT) {
			setOutput<EndOut>((float)stream.frames_available() / (float)PreBufferThreshold);
		}
	}

	// This runs in a low-priority background task:
	void async_process_filesystem() {
		using enum PlayState;

		switch (play_state) {
			case Stopped:
				break;

			case LoadSampleInfo:
				if (!stream.load(sample_filename)) {
					print_tsp("Could not load sample\n");
				}
				play_state = Stopped;
				break;

			case Reset:
				stream.seek_pos(0);
				play_state = Buffering;
				break;

			case Buffering:
			case Playing:
				if (stream.frames_available() < PreBufferThreshold) {
					if (!stream.is_eof())
						stream.read_frames_from_file();
				}
				break;
		};
	}

	void handle_play() {
		play_button.process(getState<PlayButton>() == MomentaryButton::State_t::PRESSED);
		play_jack.process(getInput<PlayTrigIn>().value_or(0));

		if (play_button.just_went_high() || play_jack.just_went_high()) {

			if (play_state == PlayState::Stopped) {
				if (stream.is_loaded()) {
					play_state = PlayState::Reset;
				}
			}

			if (play_state == PlayState::Playing) {
				play_state = PlayState::Reset;
			}
		}
	}

	void handle_load_button() {
		if (load_button.update(getState<LoadsampleAltParam>())) {
			std::string_view initial_dir = "";
			async_open_file(initial_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
				if (path) {
					sample_filename = path;
					play_state = PlayState::LoadSampleInfo;
					print_tsp("Selected file '%s' => LoadSampleInfo\n", path);
					free(path);
				}
			});
		}
	}

	void handle_loop_toggle() {
		auto loop_button = getState<LoopButton>() == MomentaryButton::State_t::PRESSED;
		auto loop_jack = getInput<LoopGateIn>().value_or(0) > 0.5f;

		// Both the button and the jack toggle loop mode ===> Button XOR Jack
		loop_mode = loop_button ^ loop_jack;

		if (loop_mode)
			setLED<LoopButton>(1, 1, 0);
		else
			setLED<LoopButton>(0, 0, 0);
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;
	}

	void load_state(std::string_view state) override {
		if (state.length()) {
			sample_filename.copy(state);
			play_state = PlayState::LoadSampleInfo;
			print_tsp("Loading file '%s' => LoadSampleInfo\n", sample_filename.c_str());
		}
	}

	std::string save_state() override {
		return std::string(sample_filename);
	}

	size_t get_display_text(int display_id, std::span<char> text) override {
		if (display_id == display_idx<ScreenOut>) {

			size_t chars_to_copy = std::min(text.size(), message.size());
			std::copy(message.data(), message.data() + chars_to_copy, text.begin());

			return chars_to_copy;

		} else {
			return 0;
		}
	}

private:
	AsyncThread fs_thread{this};

	enum class PlayState {
		Stopped,
		LoadSampleInfo,
		Buffering,
		Playing,
		Reset,
	};

	std::atomic<PlayState> play_state{PlayState::Stopped};

	StaticString<255> sample_filename = "";

	Toggler play_button;
	cpputil::SchmittTrigger play_jack{0.2f, 0.5f};

	bool loop_mode = false;

	RisingEdgeDetector load_button;

	static constexpr size_t PreBufferSamples = 1 * 1024 * 1024;
	WavFileStream<PreBufferSamples> stream;

	StaticString<255> message = "Load a Sample";

	size_t PreBufferThreshold = 1024;

	float sample_rate = 48000.f;

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
