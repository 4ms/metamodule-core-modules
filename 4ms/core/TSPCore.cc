#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "filesystem/async_filebrowser.hh"
#include "info/TSP_info.hh"
#include "tsp/wav_file_stream.hh"
#include "util/edge_detector.hh"
#include "util/oscs.hh"
#include "util/schmitt_trigger.hh"
#include "util/static_string.hh"
#include <atomic>

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
				setLED<PlayButton>(Yellow);
				if (stream.is_eof() || stream.frames_available() >= PreBufferFramesThreshold) {
					play_state = Playing;
				}
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case Playing:
				setLED<PlayButton>(Green);

				if (stream.frames_available()) {
					if (stream.is_stereo()) {
						setOutput<LeftOut>(stream.pop_sample() * 5.f);
						setOutput<RightOut>(stream.pop_sample() * 5.f);
					} else {
						setOutput<LeftOut>(stream.pop_sample() * 5.f);
						setOutput<RightOut>(0);
					}

				} else {
					// No frames avaiable.
					// If we're also at the end of file, then stop.
					// Otherwise, we have a buffer underflow, so just wait until buffer fills up
					if (stream.is_eof()) {
						end_out.start(0.010); //10ms pulse
						play_state = loop_mode ? Reset : Stopped;
					} else {
						setLED<PlayButton>(Red);
						message.copy("Buffer underflow\n");
					}
				}
				break;

			case Reset:
			case Stopped:
				setLED<PlayButton>(Off);
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case LoadSampleInfo:
				setLED<PlayButton>(Yellow);
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;
		}

		setOutput<EndOut>(end_out.update() ? 5.f : 0.f);
		setOutput<PositionOut>(8 * (float)stream.current_playback_frame() / (float)stream.total_frames());
	}

	void debug_state() {
		// setOutput<PositionOut>(5.f * (float)stream.pre_buff.head() / stream.pre_buff.max_size());
		// setOutput<PositionOut>((float)stream.frames_available() / (float)PreBufferSamples);

		// setOutput<EndOut>((float)stream.frames_available() / (float)PreBufferThreshold);
		// setOutput<EndOut>(8 * (float)stream.num_free() / (float)(PreBufferSamples));
		// setOutput<EndOut>(5.f * (float)stream.pre_buff.tail() / stream.pre_buff.max_size());
	}

	// This runs in a low-priority background task:
	void async_process_filesystem() {
		using enum PlayState;

		switch (play_state) {
			case Stopped:
				break;

			case LoadSampleInfo:
				if (!stream.load(sample_filename)) {
					message.copy("Error loading file\n");
				}
				play_state = Stopped;
				break;

			case Reset:
				stream.seek_pos(0);
				play_state = Buffering;
				break;

			case Buffering:
			case Playing:
				if (stream.frames_available() < PreBufferFramesThreshold) {
					if (!stream.is_eof()) {
						stream.read_frames_from_file();
					}
				}
				break;
		};
	}

	void handle_play() {
		play_button.process(getState<PlayButton>() == MomentaryButton::State_t::PRESSED);
		play_jack.process(getInput<PlayTrigIn>().value_or(0));

		if (play_button.just_went_high() || play_jack.just_went_high()) {

			if (play_state == PlayState::Stopped && stream.is_loaded()) {
				play_state = PlayState::Reset;
			}

			else if (play_state == PlayState::Playing)
			{
				play_state = getState<PlayRetrigModeAltParam>() ? PlayState::Stopped : PlayState::Reset;
			}
		}
	}

	void handle_load_button() {
		if (load_button.update(getState<LoadSampleAltParam>())) {
			std::string_view initial_dir = "";
			async_open_file(initial_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
				if (path) {
					load_sample(path);
					free(path);
				}
			});
		}

		PreBufferFramesThreshold = std::max(getState<PrebufferAmountAltParam>() * 4096, 1024u);
	}

	void handle_loop_toggle() {
		auto loop_button = getState<LoopButton>() == LatchingButton::State_t::DOWN;
		auto loop_jack = getInput<LoopGateIn>().value_or(0) > 0.5f;

		// Both the button and the jack toggle loop mode ===> Button XOR Jack
		loop_mode = loop_button ^ loop_jack;

		setLED<LoopButton>(loop_mode ? 0.5f : 0.f);
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;
		end_out.set_update_rate_hz(sr);
	}

	void load_state(std::string_view state) override {
		if (state.length()) {
			load_sample(state);
		}
	}

	void load_sample(std::string_view filename) {
		sample_filename.copy(filename);
		play_state = PlayState::LoadSampleInfo;
		if (auto pos = filename.find_last_of('/'); pos != filename.npos)
			message.copy(filename.substr(pos + 1, filename.length() - pos - 5));
		else
			message.copy(filename.substr(0, filename.length() - 4));
	}

	std::string save_state() override {
		return std::string(sample_filename);
	}

	size_t get_display_text(int display_id, std::span<char> text) override {
		if (display_id == display_idx<MessageDisplay>)
			return copy_text(message, text);
		else
			return 0;
	}

private:
	// File:
	AsyncThread fs_thread{this};
	StaticString<255> sample_filename = "";

	// Playing:
	enum class PlayState {
		Stopped,
		LoadSampleInfo,
		Buffering,
		Playing,
		Reset,
	};
	std::atomic<PlayState> play_state{PlayState::Stopped};

	// Controls/UI:
	Toggler play_button;
	cpputil::SchmittTrigger play_jack{0.2f, 0.5f};

	bool loop_mode = false;

	OneShot end_out{48000};

	RisingEdgeDetector load_button;

	static constexpr std::array<float, 3> Yellow = {0.9f, 0, 1.f};
	static constexpr std::array<float, 3> Red = {1.0f, 0, 0};
	static constexpr std::array<float, 3> Green = {0.1f, 1.f, 0.1f};
	static constexpr std::array<float, 3> Off = {0, 0, 0};

	StaticString<255> message = "Load a Sample";

	// Wav Stream:
	static constexpr size_t PreBufferSamples = 1 * 1024 * 1024; //~11sec stereo, 22sec mono
	size_t PreBufferFramesThreshold = 4096;						//4096 is about ~40ms latency with a fast card
	WavFileStream<PreBufferSamples> stream;

	// Resampler:
	float sample_rate = 48000.f;

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
