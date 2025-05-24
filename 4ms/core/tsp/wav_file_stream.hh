#pragma once
#include "../../medium/debug_raw.h"
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<uint64_t MaxSamples = 1024 * 1024>
struct WavFileStream {

	bool load(std::string_view sample_path) {
		unload();

		eof = false;

		// TODO: if we already have this file loaded, then just reset the read position

		loaded = drwav_init_file(&wav, sample_path.data(), nullptr);
		return loaded;
	}

	void unload() {
		reset_prebuff();

		if (loaded) {
			drwav_uninit(&wav);
			loaded = false;
		}
	}

	bool is_loaded() const {
		return loaded;
	}

	void read_frames_from_file() {
		// Read in 4kB chunks:
		unsigned frames_to_read = ReadBlockBytes / wav.fmt.blockAlign;
		read_frames_from_file(frames_to_read);
	}

	void read_frames_from_file(unsigned num_frames) {
		if (!loaded || eof)
			return;

		while (num_frames > 0) {
			// Read blocks of maximum 4kB at a time
			unsigned frames_to_read = std::min(ReadBlockBytes / wav.fmt.blockAlign, num_frames);

			last_frame_written = wav.readCursorInPCMFrames;
			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

			eof = (frames_read != frames_to_read);

			if (frames_read > num_frames) {
				printf("WavFileStream: Internal error: drwav read more frames than requested\n");
				frames_read = num_frames;
			}

			for (auto frame = 0u; frame < frames_read; frame++) {
				auto num_free = pre_buff.num_free();
				if (num_free >= wav.channels) {
					for (auto sample = 0u; sample < wav.channels; sample++) {
						pre_buff.put(read_buff[frame * wav.channels + sample]);
					}
				} else {
					printf("WavFileStream: prebuffer overflow\n");
					break;
				}

				num_frames--;
				last_frame_written++;
			}

			if (eof)
				break;
		}
	}

	float pop_sample() {
		auto p = pre_buff.get().value_or(0);
		return p;
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
		return last_frame_written > frames_available() ? last_frame_written - frames_available() : 0;
	}

	uint64_t total_frames() const {
		return loaded ? wav.totalPCMFrameCount : 0;
	}

	void seek_frame_in_file(uint64_t frame_num = 0) {

		const auto frames_in_prebuff = std::min(MaxSamples / wav.channels, last_frame_written);

		// Optimization:
		// if we request to seek to a frame that's already in the prebuffer,
		// just jump the read head to there (no need to read from disk)
		if (frame_num < last_frame_written && frame_num >= (last_frame_written - frames_in_prebuff)) {
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
		last_frame_written = 0;
	}

	drwav wav;

	bool eof = true;
	bool loaded = false;

	uint64_t last_frame_written = 0;

public:
	LockFreeFifoSpsc<float, MaxSamples> pre_buff;

private:
	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	static constexpr unsigned ReadBlockBytes = 4096;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// e.g. 4kB of 8-bit mono will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;
};

} // namespace MetaModule
