#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "dsp/stream_resampler.hh"
#include "filesystem/async_filebrowser.hh"
#include "graphics/waveform_display.hh"
#include "gui/notification.hh"
#include "info/TSP_info.hh"
#include "patch/patch_file.hh"
#include "util/edge_detector.hh"
#include "util/oscs.hh"
#include "util/schmitt_trigger.hh"
#include "util/static_string.hh"
#include "wav/wav_file_stream.hh"
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
		waveform.set_cursor_width(2);
	}

	~TSPCore() {
		fs_thread.stop();
		stream.unload();
	}

	void update() override {
		handle_play();
		handle_loop_toggle();
		handle_zoom();

		auto current_frame = stream.current_playback_frame();

		// default output values:
		setOutput<LeftOut>(0);
		setOutput<RightOut>(0);

		setOutput<EndOut>(end_out.update() ? 5.f : 0.f);
		setOutput<PositionOut>(5.f * (float)current_frame / (float)stream.total_frames());

		// Clear error messages after a moment
		if (err_message.length() > 0 && !error_message_hold.update()) {
			err_message = "";
		}

		switch (play_state) {
			using enum PlayState;
			case Buffering:
				if (stream.is_eof() || stream.frames_available() >= prebuff_threshold) {
					err_message.clear();
					play_state = Playing;
				}
				setLED<PlayButton>(Yellow);
				break;

			case Playing:
				// Loop and EndOut:
				if (current_frame >= stream.total_frames()) {
					end_out.start(0.010);
					stream.reset_playback_to_frame(0);
					if (!loop_mode)
						play_state = Stopped;
				}

				if (stream.frames_available() > 0) {
					setLED<PlayButton>(Green);
					auto [left, right] = resampler.process_stereo([this] { return stream.pop_sample(); });

					setOutput<LeftOut>(left * 5.f);
					setOutput<RightOut>(right * 5.f);

					waveform.draw_sample(left);
					waveform.set_cursor_position((float)current_frame / stream.total_frames());

				} else {
					if (!stream.is_eof() && err_message.length() == 0) {
						setLED<PlayButton>(Red);
						Gui::notify_user("TSP: Buffer underflow", 1000);
						err_message = "Underflow";
						error_message_hold.start(1); //2 seconds
					}
				}
				break;

			case Stopped:
			case Restart:
				stream.reset_playback_to_frame(0);
				waveform.set_cursor_position(0);
				setLED<PlayButton>(Off);
				break;

			case Paused:
				setLED<PlayButton>(Off);
				break;

			case LoadSampleInfo:
				setLED<PlayButton>(Yellow);
				break;
		}
	}

	// This runs in a low-priority background task:
	void async_process_filesystem() {
		handle_load_button();

		setLED<BusyLight>(0.f);

		using enum PlayState;

		switch (play_state) {
			case Stopped:
				break;

			case LoadSampleInfo:
				if (!stream.load(sample_filename)) {
					err_message = "Error loading file";
				}
				resampler.set_sample_rate_in_out(stream.wav_sample_rate().value_or(sample_rate), sample_rate);
				resampler.set_num_channels(stream.is_stereo() ? 2 : 1);
				resampler.flush();
				play_state = Paused;
				break;

			case Restart:
				waveform.sync();
				stream.seek_frame_in_file(0);
				play_state = Buffering;
				break;

			case Paused:
			case Buffering:
			case Playing:
				if (stream.frames_available() < prebuff_threshold) {

					if (stream.is_eof()) {
						if (loop_mode) {
							//TODO: try play_state = Restart;
							stream.seek_frame_in_file(0);
						}
					} else {
						setLED<BusyLight>(1.f);
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
				play_state = PlayState::Restart;
			}

			else if (play_state == PlayState::Paused && stream.is_loaded())
			{
				play_state = PlayState::Playing;
			}

			else if (play_state == PlayState::Playing)
			{
				if (getState<PlayRetrigModeAltParam>() == RetrigMode::Stop) {
					end_out.start(0.010);
					play_state = PlayState::Stopped;

				} else if (getState<PlayRetrigModeAltParam>() == RetrigMode::Retrigger) {
					end_out.start(0.010);
					play_state = PlayState::Restart;

				} else {
					play_state = PlayState::Paused;
				}
			}
		}
	}

	void handle_load_button() {
		// Set initial state
		if (load_button == -1)
			load_button = getState<LoadSampleAltParam>();

		// Change of value => perform action
		if (load_button != getState<LoadSampleAltParam>()) {
			load_button = getState<LoadSampleAltParam>();

			std::string_view initial_dir = "";
			async_open_file(initial_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
				if (path) {
					load_sample(path);
					free(path);
					Patch::mark_patch_modified();
				}
			});
		}
	}

	void handle_zoom() {
		float zoom = 300.f * getState<WaveformZoomAltParam>() + 1.f; //1..301
		waveform.set_x_zoom(zoom);
	}

	void set_param(int id, float val) override {
		// Buffer Threshold alt param: handle it only when the user changes it
		if (id == param_idx<BufferThresholdAltParam>) {
			auto samples_per_frame = stream.is_stereo() ? 2 : 1;
			auto max_frames = std::min(stream.total_frames(), MByteToSamples(buffer_size_mb) / samples_per_frame);

			prebuff_threshold = std::max<unsigned>(val * 0.8f * max_frames, 1024);
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
		error_message_hold.set_update_rate_hz(sample_rate);

		if (auto source_sr = stream.wav_sample_rate()) {
			resampler.set_sample_rate_in_out(*source_sr, sample_rate);
		}
	}

	void load_sample(std::string_view filename) {
		sample_filename.copy(filename);
		play_state = PlayState::LoadSampleInfo;
		if (auto pos = filename.find_last_of('/'); pos != filename.npos)
			wav_name.copy(filename.substr(pos + 1, filename.length() - pos - 5));
		else
			wav_name.copy(filename.substr(0, filename.length() - 4));
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
		if (display_id == display_idx<MessageDisplay>) {
			return copy_text(err_message.length() ? err_message : wav_name, text);
		} else
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
	StaticString<255> err_message = "";
	StaticString<255> wav_name = "Load a Sample";

	enum class PlayState { Stopped, LoadSampleInfo, Buffering, Playing, Paused, Restart };
	std::atomic<PlayState> play_state{PlayState::Stopped};

	Toggler play_button;
	cpputil::SchmittTrigger play_jack{0.2f, 0.5f};

	RisingEdgeDetector loop_button;
	cpputil::SchmittTrigger loop_jack{0.2f, 0.5f};
	bool loop_mode = false;

	OneShot end_out{48000};

	OneShot error_message_hold{48000};

	float load_button = -1;

	static constexpr std::array<float, 3> Yellow = {0.9f, 1.f, 0};
	static constexpr std::array<float, 3> Red = {1.0f, 0, 0};
	static constexpr std::array<float, 3> Green = {0.1f, 1.f, 0.1f};
	static constexpr std::array<float, 3> Off = {0, 0, 0};

	StreamingWaveformDisplay waveform{
		base_element(WaveformDisplay).width_mm,
		base_element(WaveformDisplay).height_mm,
	};

	float sample_rate = 48000.f;
	unsigned prebuff_threshold = 1024;

	// Sizes of pre-buffer. This determines how much sample data we store in RAM.
	// The larger this is, the longer the sample we can load and re-trigger without
	// needing to read from disk again.
	// 1MB is 256k samples, or about 2.7sec of stereo or 5.5sec of mono
	// 2MB is 512k samples, or about 5.5sec of stereo or 10.9sec of mono
	// 4MB is 1M samples, or about 10.9sec of stereo or 21.8sec of mono
	// 8MB is 2M samples, or about 21.8sec of stereo or 43.7sec of mono
	// 16MB is 4M samples, or about 43.7sec of stereo or 87.4sec of mono

#ifdef VCVRACK
	unsigned buffer_size_mb = 32;
#else
	unsigned buffer_size_mb = 8;
#endif

	constexpr unsigned MByteToSamples(unsigned MBytes) {
		return MBytes * 1024 * 1024 / 4;
	}

	WavFileStream stream{MByteToSamples(buffer_size_mb)};

	StreamResampler resampler{2}; //2: stereo

	enum RetrigMode { Retrigger = 0, Stop = 1, Pause = 2 };

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
