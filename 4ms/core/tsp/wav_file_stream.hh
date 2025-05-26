#pragma once
// #include "../../medium/debug_raw.h"
#include "dsp/resampler.hh"
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<uint64_t MaxSamples = 1024 * 1024, uint32_t MaxResamplingRatio = 4>
struct WavFileStream {

	bool load(std::string_view sample_path) {
		unload();

		eof = false;

		loaded = drwav_init_file(&wav, sample_path.data(), nullptr);
		return loaded;
	}

	void unload() {
		reset_prebuff();

		resampler.flush();

		if (loaded) {
			drwav_uninit(&wav);
			loaded = false;
		}
	}

	bool is_loaded() const {
		return loaded;
	}

	void read_frames_from_file() {
		unsigned frames_to_read = ReadBlockBytes / wav.fmt.blockAlign;
		read_frames_from_file(frames_to_read);
	}

	void read_frames_from_file(unsigned num_frames) {
		if (!loaded || eof)
			return;

		while (num_frames > 0) {
			// Read blocks of maximum 4kB at a time
			unsigned frames_to_read = std::min(ReadBlockBytes / wav.fmt.blockAlign, num_frames);

			next_frame_to_write = wav.readCursorInPCMFrames;
			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

			eof = (frames_read != frames_to_read);

			if (frames_read > frames_to_read) {
				printf("WavFileStream: Internal error: drwav read more frames than requested\n");
				frames_read = frames_to_read;
			}

			// Push samples into resampler
			resampler.set_samplerate_in_out(wav.sampleRate, out_sr);

			auto input = std::span<const float>(read_buff.data(), frames_read * wav.channels);
			auto output = resampler.process_block(wav.channels, input);

			for (auto out : output) {
				if (!pre_buff.put(out)) {
					printf("WavFileStream: Buffer overflow\n");
					// TODO: Handle buffer overflow.
					// Set drwav read cursor back to this position, pop back if we're not a frame boundary,
					// set next_frame_to_write, and abort
				}
			}
			num_frames -= output.size() / wav.channels;
			next_frame_to_write += output.size() / wav.channels;

			if (eof)
				break;
		}
	}

	float pop_sample() {
		auto p = pre_buff.get().value_or(0);
		return p;
	}

	void set_samplerate(float samplerate) {
		out_sr = samplerate;
		if (loaded) {
			resampler.set_samplerate_in_out(wav.sampleRate, out_sr);
		}
	}

	bool is_stereo() const {
		return loaded ? wav.channels > 1 : false;
	}

	unsigned samples_available() const {
		return pre_buff.num_filled();
	}

	unsigned frames_available() const {
		return samples_available() / wav.channels;
	}

	bool is_eof() const {
		return eof;
	}

	uint64_t current_playback_frame() const {
		return next_frame_to_write > frames_available() ? next_frame_to_write - frames_available() : 0;
	}

	uint64_t total_frames() const {
		return loaded ? wav.totalPCMFrameCount : 0;
	}

	void seek_frame_in_file(uint64_t frame_num = 0) {

		const auto frames_in_prebuff = std::min(MaxSamples / wav.channels, next_frame_to_write);

		// Optimization:
		// if we request to seek to a frame that's already in the prebuffer,
		// just jump the read head to there (no need to read from disk)
		if (frame_num < next_frame_to_write && frame_num >= (next_frame_to_write - frames_in_prebuff)) {
			pre_buff.set_read_pos(frame_num * wav.channels);
		} else {
			// Otherwise, we don't have the requested frame in our buffer
			// so we need to prepare to read from disk
			drwav_seek_to_pcm_frame(&wav, frame_num);

			reset_prebuff();

			eof = false;
		}
	}

	std::optional<uint32_t> wav_sample_rate() const {
		if (loaded)
			return wav.sampleRate;
		else
			return {};
	}

private:
	void reset_prebuff() {
		pre_buff.set_write_pos(0);
		pre_buff.set_read_pos(0);
		next_frame_to_write = 0;
	}

	drwav wav;

	bool eof = true;
	bool loaded = false;

	uint64_t next_frame_to_write = 0;

	LockFreeFifoSpsc<float, MaxSamples> pre_buff;

	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	static constexpr unsigned ReadBlockBytes = 4096;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// Worst case: 4kB of 8-bit mono data will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;

	static constexpr auto MaxChannels = 2; //Stereo or Mono files only
	ResamplingInterleavedBuffer<MaxChannels, ReadBlockBytes, MaxResamplingRatio> resampler;
	// AudioResampler resampler{2};
	// std::array<float, ReadBlockBytes * 2> resamp_buff;
	float out_sr = 48000.f;
};

} // namespace MetaModule
