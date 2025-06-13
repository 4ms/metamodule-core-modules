#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "dsp/resampler_one.hh"
#include "filesystem/async_filebrowser.hh"
#include "graphics/waveform_display.hh"
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

		waveform.set_wave_color(0x33, 0xFF, 0xBB); //teal
		waveform.set_bar_color(0x55, 0x55, 0x55);  //dark grey
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

		auto current_frame = stream.current_playback_frame();

		switch (play_state) {
			case Buffering:
				if (stream.is_eof() || stream.frames_available() >= prebuff_threshold) {
					play_state = Playing;
				}
				setLED<PlayButton>(Yellow);
				setOutput<LeftOut>(0);
				setOutput<RightOut>(0);
				break;

			case Playing:
				setLED<PlayButton>(Green);

				if (loop_mode && current_frame == 1) {
					end_out.start(0.010);
				}

				if (stream.frames_available() > 16) {
					float out[2];
					resampler.process([this] { return stream.pop_sample(); }, out);
					// auto [left, right] = resampler.pop([this] { return stream.pop_sample(); });

					setOutput<LeftOut>(out[0] * 5.f);
					setOutput<RightOut>(out[1] * 5.f);

					waveform.draw_sample(out[0]);
					waveform.set_cursor_position((float)current_frame / stream.total_frames());

				} else {
					// No frames avaiable.
					// If we're also at the end of file, then stop or loop.
					// Otherwise, we have a buffer underflow, so just wait until buffer fills up
					if (stream.is_eof()) {
						end_out.start(0.010);
						play_state = Stopped;
					} else {
						setLED<PlayButton>(Red);
						message = "Buffer underflow";
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
		setOutput<PositionOut>(5.f * (float)current_frame / (float)stream.total_frames());
	}

	// This runs in a low-priority background task:
	void async_process_filesystem() {
		using enum PlayState;

		switch (play_state) {
			case Stopped:
				break;

			case LoadSampleInfo:
				if (!stream.load(sample_filename)) {
					message = "Error loading file";
				}
				resampler.set_num_channels(stream.is_stereo() ? 2 : 1);
				resampler.flush();
				play_state = Stopped;
				break;

			case Reset:
				stream.seek_frame_in_file(0);
				play_state = Buffering;
				break;

			case Buffering:
			case Playing:
				if (stream.frames_available() < prebuff_threshold) {

					if (stream.is_eof()) {
						if (loop_mode) {
							stream.seek_frame_in_file(0);
						}
					} else {
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
			waveform.sync();

			if (play_state == PlayState::Stopped && stream.is_loaded()) {
				play_state = PlayState::Reset;
			}

			else if (play_state == PlayState::Playing)
			{
				end_out.start(0.010);
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

		float inc = 1.f / (300.f * getState<WaveformZoomAltParam>() + 1.f);
		waveform.set_x_zoom(inc);
	}

	void set_param(int id, float val) override {
		// Buffer Threshold alt param: handle it only when the user changes it
		if (id == param_idx<BufferThresholdAltParam>) {
			const auto samples_per_frame = stream.is_stereo() ? 2 : 1;
			const auto max_frames = std::min<unsigned>(stream.total_frames(), PreBufferSamples / samples_per_frame);

			prebuff_threshold = std::max<unsigned>(val * 0.8f * max_frames, 1024);

			// printf("prebuff_threshold = %u, frames avail = %u, total_frames = %u, max_frames = %u\n",
			// 	   prebuff_threshold,
			// 	   stream.frames_available(),
			// 	   stream.total_frames(),
			// 	   max_frames);
		}

		// All other parameters:
		SmartCoreProcessor::set_param(id, val);
	}

	void handle_loop_toggle() {
		loop_mode = getState<LoopButton>() == LatchingButton::State_t::DOWN;

		loop_jack.process(getInput<LoopGateIn>().value_or(0));
		if (loop_jack.is_high()) {
			loop_mode = !loop_mode;
		}

		setLED<LoopButton>(loop_mode ? 1.f : 0.f);
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;

		end_out.set_update_rate_hz(sample_rate);

		if (auto source_sr = stream.wav_sample_rate()) {
			resampler.set_sample_rate_in_out(*source_sr, sample_rate);
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

	void load_state(std::string_view state) override {
		if (state.length()) {
			load_sample(state);
		}
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

	void show_graphic_display(int display_id, std::span<uint32_t> buf, unsigned width, lv_obj_t *canvas) override {
		if (display_id == display_idx<WaveformDisplay>)
			waveform.show_graphic_display(buf, width, canvas);
	}

	bool draw_graphic_display(int display_id) override {
		if (display_id == display_idx<WaveformDisplay>)
			return waveform.draw_graphic_display();
		else
			return false;
	}

	void hide_graphic_display(int display_id) override {
		if (display_id == display_idx<WaveformDisplay>)
			waveform.hide_graphic_display();
	}

private:
	AsyncThread fs_thread{this};
	StaticString<255> sample_filename = "";

	enum class PlayState {
		Stopped,
		LoadSampleInfo,
		Buffering,
		Playing,
		Reset,
	};
	std::atomic<PlayState> play_state{PlayState::Stopped};

	Toggler play_button;
	cpputil::SchmittTrigger play_jack{0.2f, 0.5f};

	RisingEdgeDetector loop_button;
	cpputil::SchmittTrigger loop_jack{0.2f, 0.5f};
	bool loop_mode = false;

	OneShot end_out{48000};

	RisingEdgeDetector load_button;

	static constexpr std::array<float, 3> Yellow = {0.9f, 1.f, 0};
	static constexpr std::array<float, 3> Red = {1.0f, 0, 0};
	static constexpr std::array<float, 3> Green = {0.1f, 1.f, 0.1f};
	static constexpr std::array<float, 3> Off = {0, 0, 0};

	StaticString<255> message = "Load a Sample";

	StreamingWaveformDisplay waveform{
		base_element(WaveformDisplay).width_mm,
		base_element(WaveformDisplay).height_mm,
	};

	float sample_rate = 48000.f;
	unsigned prebuff_threshold = 1024;

	// Size of pre-buffer. This determines how much sample data we store in RAM
	// The larger this is, the longer the sample we can load and re-trigger without
	// needing to read from disk again.
	// 512k samples is about 5.5sec of stereo or 11sec of mono and uses 512k * 4 = 2MB
	// 1024K samples is about 11sec of stereo or 22sec of mono and uses 1M * 4 = 4MB
	static constexpr size_t PreBufferSamples = 1024 * 1024;

	// Maximum resampling ratio we support. Worse case is reading a 22k file and
	// playing back at 96kHz
	static constexpr unsigned MaxResampleRatio = (96000.f / 22050.f) + 1; // +1 to round up

	WavFileStream<PreBufferSamples> stream;

	Resampler<2> resampler{2};

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
