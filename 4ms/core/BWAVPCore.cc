#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/register_module.hh"
#include "dsp/stream_resampler.hh"
#include "filesystem/async_filebrowser.hh"
#include "filesystem/helpers.hh"
#include "graphics/waveform_display.hh"
#include "gui/notification.hh"
#include "patch/patch_file.hh"
#include "system/time.hh"
#include "util/edge_detector.hh"
#include "util/oscs.hh"
#include "util/schmitt_trigger.hh"
#include "util/static_string.hh"
#include "wav/wav_file_stream.hh"
#include <atomic>

#include "info/BWAVP_info.hh"

namespace MetaModule
{

#ifdef VCVRACK
namespace Async
{
void start_module_threads();
}
#endif

class BWAVPCore : public SmartCoreProcessor<BWAVPInfo> {
	using enum BWAVPInfo::Elem;
	enum class PlayState { Stopped, LoadSampleInfo, Buffering, Playing, Paused, Restart, FileError };

public:
	BWAVPCore() {
		fs_thread.start([this]() { async_process_filesystem(); });

		waveform.set_wave_color(Teal);
		waveform.set_bar_bg_color(Black);
		waveform.set_bar_fg_color(LightGrey);
		waveform.set_cursor_width(2);
#ifdef VCVRACK
		// Lazy init threads to reduce impact on Rack app startup/quit
		Async::start_module_threads();
#endif
	}

	~BWAVPCore() {
		fs_thread.stop();
		stream.unload();
	}

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		handle_play();

		auto current_frame = stream.current_playback_frame();

		// default output values:
		float outL = 0;
		float outR = 0;

		setOutput<EndOut>(end_out.update() ? 5.f : 0.f);
		setOutput<PlayGateOut>(play_state == PlayState::Playing ? 5 : 0);

		switch (play_state) {
			using enum PlayState;

			case Buffering:
				if (stream.is_eof() || stream.frames_available() >= playback_threshold_frames()) {
					err_message.clear();
					play_state = Playing;
				}
				break;

			case Playing:
				// Loop and EndOut:
				if (current_frame >= stream.total_frames()) {
					end_out.start(0.010);
					stream.reset_playback_to_frame(0);
					if (!loop_mode) {
						play_state = Stopped;
					}
				}

				if (stream.frames_available() > 0) {
					auto [left, right] = resampler.process_stereo([this] { return stream.pop_sample(); });
					outL = left * 5.f;
					outR = right * 5.f;

					waveform.draw_sample(left);
					waveform.set_cursor_position((float)current_frame / stream.total_frames());

					err_message = "";

				} else {
					if (!stream.is_eof() && err_message.length() == 0) {
						setLED<PlayButton>(Red);
						waveform.set_wave_color(Red);
						Gui::notify_user("Sample buffer underflow", 3000);
						err_message = "Underflow";
					}
				}

				break;

			case FileError:
				if (file_error_retry.update() == false) {
					next_play_state = Buffering;
					play_state = LoadSampleInfo;
				}
				break;

			case Stopped:
			case Restart:
			case Paused:
			case LoadSampleInfo:
				break;
		}

		setOutput<LeftOut>(outL);
		setOutput<RightOut>(outR);
	}

	// This runs in a low-priority background task:
	void async_process_filesystem() {
		using enum PlayState;

		setLED<BusyLight>(0.f);

		handle_loop_toggle();
		handle_resize_buffer();

		switch (play_state) {
			case Stopped:
			case FileError:
				break;

			case LoadSampleInfo:
				if (!stream.load(sample_filename)) {
					err_message = "Error loading file";
					play_state = FileError;
					file_error_retry.start(0.5f);
				} else {
					auto sr = stream.wav_sample_rate();
					resampler.set_sample_rate_in_out(sr ? sr : sample_rate, sample_rate);
					resampler.set_num_channels(stream.num_channels());
					resampler.flush();
					err_message.clear();
					display_sample_name();
					play_state.store(next_play_state);
				}
				break;

			case Restart:
				// Playback is disabled, audio thread is not touching any shared data
				stream.reset_playback_to_frame(0);
				waveform.set_cursor_position(0);
				waveform.sync();
				stream.seek_frame_in_file(0);
				play_state.store(next_play_state.load());
				break;

			case Paused:
			case Buffering:
			case Playing:
				if (delayed_start_time <= System::get_ticks()) {
					if (stream.frames_available() < prebuff_threshold_frames()) {
						if (stream.is_eof()) {
							if (loop_mode) {
								stream.seek_frame_in_file(0);
							}
						} else {
							setLED<BusyLight>(1.f);
							stream.read_frames_from_file();
						}
					}
				}

				break;
		};

		if (stream.is_file_error()) {
			err_message = "Disk Error";
			play_state.store(FileError, std::memory_order_seq_cst);
			stream.unload();
			file_error_retry.start(0.5f);
		}

		// Draw waveform and buffer bar

		waveform.set_x_zoom(300.f * getState<WaveformZoomAltParam>() + 1.f);

		if (stream.is_eof() && stream.first_frame_in_buffer() == 0)
			waveform.set_bar_fg_color(DarkGreen);
		else if (!stream.is_eof() && (stream.frames_available() + 1024) < playback_threshold_frames())
			waveform.set_bar_fg_color(LightGrey);
		else
			waveform.set_bar_fg_color(Blue);

		if (delayed_start_time > System::get_ticks())
			waveform.set_wave_color(Blue);

		else if (play_state == Playing) {
			setLED<PlayButton>(Green);
			waveform.set_wave_color(Teal);

		} else if (play_state == Paused) {
			setLED<PlayButton>(Off);
			waveform.set_wave_color(Teal);

		} else if (play_state == FileError) {
			setLED<PlayButton>(Red);
			waveform.set_wave_color(Red);

		} else if (play_state == LoadSampleInfo) {
			setLED<PlayButton>(Yellow);
			waveform.set_wave_color(Yellow);

		} else if (play_state == Buffering) {
			setLED<PlayButton>(Yellow);
			waveform.set_wave_color(Teal);

		} else if (play_state == Stopped || play_state == Restart) {
			setLED<PlayButton>(Off);
		}

		float total = stream.total_frames();
		waveform.set_bar_begin_end(total ? (float)stream.first_frame_in_buffer() / total : 0,
								   total ? (float)stream.latest_buffered_frame() / total : 0);
	}

	void handle_play() {
		play_button.process(getState<PlayButton>() == MomentaryButton::State_t::PRESSED);
		play_jack.process(getInput<PlayTrigIn>().value_or(0));

		if (play_button.just_went_high() || play_jack.just_went_high()) {
			if (stream.is_loaded()) {

				if (play_state == PlayState::Stopped) {
					restart_playback(PlayState::Buffering);

				} else if (play_state == PlayState::Paused) {
					play_state = PlayState::Buffering;

				} else if (play_state == PlayState::Playing || play_state == PlayState::Buffering) {
					if (getState<PlayRetrigModeAltParam>() == RetrigMode::Stop) {
						end_out.start(0.010);
						restart_playback(PlayState::Paused);

					} else if (getState<PlayRetrigModeAltParam>() == RetrigMode::Retrigger) {
						end_out.start(0.010);
						restart_playback(PlayState::Buffering);

					} else { // RetrigMode::Pause
						play_state = PlayState::Paused;
					}
				}
			}
		}
	}

	void handle_load_button() {
		std::string_view initial_dir{sample_filename};
		initial_dir = initial_dir.substr(0, initial_dir.find_last_of("/"));
		async_open_file(initial_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
			if (path) {
				load_sample(path);
				free(path);
				Patch::mark_patch_modified();
			}
		});
	}

	void restart_playback(PlayState next) {
		// Tell async thread to seek frame 0, and then move to the next play state
		next_play_state = next;
		play_state = PlayState::Restart;
		// stream.reset_playback_to_frame(0);
		// waveform.set_cursor_position(0);
	}

	void set_param(int id, float val) override {
		SmartCoreProcessor::set_param(id, val);

		// Load Sample: process it and then set the param back to 0
		if (id == param_idx<LoadSampleAltParam> && val == 1) {
			handle_load_button();
			SmartCoreProcessor::set_param(id, 0);
		}
		// Startup Delay
		if (id == param_idx<StartupDelay_Sec_AltParam> && delayed_start_time == 0) {
			auto delayed_param = getState<StartupDelay_Sec_AltParam>();
			delayed_start_time = System::get_ticks() + (StartupDelaySecs[delayed_param] * 1000);
		}
	}

	void handle_resize_buffer() {
		using enum PlayState;
		if (unsigned new_size_idx = getState<MaxBufferSizeAltParam>(); new_size_idx < BufferSizes.size()) {

			auto new_size_samples = MByteToSamples(BufferSizes[new_size_idx]);
			if (new_size_samples != stream.max_size()) {
				// Stop playback before changing buffer size to avoid race conditions
				auto prev_state = play_state.load(std::memory_order_seq_cst);
				play_state.store(Stopped, std::memory_order_seq_cst);

				stream.resize(new_size_samples);

				// Restart playback if we were playing before changing buffer size
				if (prev_state == LoadSampleInfo) {
					next_play_state = Paused;
					play_state = LoadSampleInfo;

				} else if (prev_state == Playing || prev_state == Buffering) {
					restart_playback(PlayState::Buffering);
				}
			}
		}
	}

	void handle_loop_toggle() {
		loop_mode = getState<LoopButton>() == LatchingButton::State_t::DOWN;

		loop_jack.process(getInput<LoopGateIn>().value_or(0));
		if (loop_jack.is_high()) {
			loop_mode = !loop_mode;
		}

		setLED<LoopButton>(loop_mode ? 1.f : 0.f);
	}

	unsigned playback_threshold_frames() {
		auto max_frames = stream.buffer_frames();
		if (max_frames <= 1024)
			return max_frames;

		uint32_t threshold = getState<PlaybackBufferThresholdAltParam>() * max_frames;

		// Don't allow making threshold at or near 100% if the sample cannot be
		// fully buffered, or else we get too many frequent small disk reads
		if (threshold > (max_frames - 1024)) {
			if (stream.total_frames() > max_frames) {
				threshold = max_frames - 1024;
			}
		}
		return std::max<unsigned>(threshold, 1024);
	}

	unsigned prebuff_threshold_frames() {
		return getState<BufferStrategyAltParam>() == 0 ? playback_threshold_frames() : stream.buffer_frames();
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;

		end_out.set_update_rate_hz(sample_rate);
		file_error_retry.set_update_rate_hz(sample_rate);

		if (auto source_sr = stream.wav_sample_rate(); source_sr > 0) {
			resampler.set_sample_rate_in_out(source_sr, sample_rate);
		}
	}

	void load_sample(std::string_view filename) {
		sample_filename.copy(filename);
		next_play_state = (play_state == PlayState::Playing) ? PlayState::Buffering : PlayState::Paused;
		play_state = PlayState::LoadSampleInfo;
	}

	void display_sample_name() {
		std::string_view filename{sample_filename};
		if (auto pos = filename.find_last_of('/'); pos != filename.npos)
			wav_name.copy(filename.substr(pos + 1, filename.length() - pos - 5));
		else
			wav_name.copy(filename.substr(0, filename.length() - 4));

		auto secs = static_cast<unsigned>(std::round(stream.sample_seconds()));
		if (secs < 60)
			snprintf(wav_name.end(), wav_name.available(), " (%us)", secs);
		else
			snprintf(wav_name.end(), wav_name.available(), " (%um%us)", secs / 60, secs % 60);
	}

	void load_state(std::string_view state) override {
		handle_resize_buffer();
		if (state.length()) {
			auto local_path = Filesystem::translate_path_to_local(state, Patch::get_dir());
			load_sample(local_path);
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

	std::atomic<PlayState> play_state{PlayState::Paused};
	std::atomic<PlayState> next_play_state{PlayState::Paused};

	uint32_t delayed_start_time = 0;

	Toggler play_button;
	cpputil::SchmittTrigger play_jack{0.2f, 0.5f};

	RisingEdgeDetector loop_button;
	cpputil::SchmittTrigger loop_jack{0.2f, 0.5f};
	bool loop_mode = false;

	OneShot end_out{48000};

	OneShot file_error_retry{48000};

	StreamingWaveformDisplay waveform{
		base_element(WaveformDisplay).width_mm,
		base_element(WaveformDisplay).height_mm,
	};

	float sample_rate = 48000.f;

	static constexpr unsigned MByteToSamples(unsigned MBytes) {
		return MBytes * 1024u * 1024 / 2; //assume 16-bit samples
	}
	static constexpr std::array<unsigned, 12> BufferSizes{1, 2, 4, 8, 16, 24, 32, 48, 64, 80, 96, 128};
	static constexpr std::array<unsigned, 21> StartupDelaySecs{0,  1,  2,  3,  4,  5,	8,	 10,  12,  15, 20,
															   25, 30, 45, 60, 90, 120, 150, 180, 240, 300};

	unsigned default_buffer_size_mb = 8;
	WavFileStream stream{MByteToSamples(default_buffer_size_mb)};

	StreamResampler resampler{2}; //2: stereo

	enum RetrigMode { Retrigger = 0, Stop = 1, Pause = 2 };

	static constexpr std::array<float, 3> Teal = {0.2f, 1.f, 0.73f};
	static constexpr std::array<float, 3> Yellow = {0.9f, 0.8f, 0};
	static constexpr std::array<float, 3> Red = {1.0f, 0, 0};
	static constexpr std::array<float, 3> Green = {0.0f, 1.f, 0.0f};
	static constexpr std::array<float, 3> Blue = {0.f, 0.3f, 0.85f};
	static constexpr std::array<float, 3> DarkGreen = {0.1f, 0.7f, 0.35f};
	static constexpr std::array<float, 3> LightGrey = {0.5f, 0.5f, 0.5f};
	static constexpr std::array<float, 3> Black = {0.f, 0.f, 0.f};
	static constexpr std::array<float, 3> Orange = {1.0f, 0.5f, 0.1f};
	static constexpr std::array<float, 3> Off = {0, 0, 0};

	static inline bool was_registered = register_module<BWAVPCore, BWAVPInfo>("4msCompany");
};

} // namespace MetaModule
