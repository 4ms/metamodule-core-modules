#pragma once
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<int64_t MaxSamples = 1024 * 1024>
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

	bool is_loaded() {
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

		while (!eof && num_frames > 0) {
			// Don't try to read more than 4kB at a time
			unsigned frames_to_read = std::min(ReadBlockBytes / wav.fmt.blockAlign, num_frames);

			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

			// printf("Read %llu frames => file position is now %llu\n", frames_read, wav.readCursorInPCMFrames);

			eof = (frames_read != frames_to_read);

			for (auto sample : std::span<float>(read_buff.begin(), frames_read * wav.channels)) {
				if (!pre_buff.put(sample)) {
					printf("WavFileStream: buffer overflow\n");
					break;
				}
				samples_written_to_prebuff++;
			}

			num_frames -= frames_read;
		}
	}

	float pop_sample() {
		return pre_buff.get().value_or(0);
	}

	bool is_stereo() const {
		return wav.channels > 1;
	}

	unsigned samples_available() {
		return pre_buff.num_filled();
	}

	unsigned frames_available() {
		return samples_available() / wav.channels;
	}

	bool is_eof() {
		return eof;
	}

	void seek_pos(int64_t frame_num = 0) {

		// Optimization:
		// if we request to seek to a frame that's already in the prebuffer,
		// just jump the read head to there (no need to read from disk)
		if (frame_num < samples_written_to_prebuff && frame_num > (samples_written_to_prebuff - MaxSamples)) {
			pre_buff.set_read_pos(frame_num * wav.channels);
		} else {
			// Otherwise, we don't have the requested frame in our buffer
			// so we need to prepare to read from disk
			drwav_seek_to_pcm_frame(&wav, frame_num);

			// FIXME: samples_written_to_prebuff is not accurate if frame_num is not 0!
			reset_prebuff();

			eof = false;
		}
	}

	void reset_prebuff() {
		pre_buff.set_write_pos(0);
		pre_buff.set_read_pos(0);
		samples_written_to_prebuff = 0;
	}

private:
	drwav wav;

	bool eof = true;
	bool loaded = false;
	int64_t samples_written_to_prebuff = 0;

	LockFreeFifoSpsc<float, MaxSamples> pre_buff;

	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	static constexpr unsigned ReadBlockBytes = 4096;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// e.g. 4kB of 8-bit mono will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;
};

} // namespace MetaModule
