#pragma once
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<size_t MaxSamples = 1024 * 1024>
struct WavFileStream {

	bool load(std::string_view sample_path) {
		unload();

		eof = false;

		// TODO: if we already have this file loaded, then just reset the read position

		loaded = drwav_init_file(&wav, sample_path.data(), nullptr);
		return loaded;
	}

	void unload() {
		pre_buff.reset();

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

		while (num_frames > 0) {
			// Don't try to read more than 4kB at a time
			unsigned frames_to_read = std::min(ReadBlockBytes / wav.fmt.blockAlign, num_frames);

			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

			if (canary[0] != 0xAAAA5555 || canary[1] != 0x12345678 || canary[2] != 0xBADCAFE0 ||
				canary[3] != 0x600DCAFE)
			{
				printf("WavFileStream: overran internal read buffer: %x %x %x %x\n",
					   canary[0],
					   canary[1],
					   canary[2],
					   canary[3]);
				printf("Req: %u, read: %llu, blockalign: %u\n", num_frames, frames_read, wav.fmt.blockAlign);
			}

			// printf("Read %llu frames => file position is now %llu\n", frames_read, wav.readCursorInPCMFrames);

			eof = (frames_read != frames_to_read);

			for (auto sample : std::span<float>(read_buff.begin(), frames_read * wav.channels)) {
				if (!pre_buff.put(sample)) {
					printf("WavFileStream: buffer overflow\n");
					break;
				}
			}

			num_frames -= frames_read;

			if (eof)
				break;
		}
	}

	float pop_sample() {
		return pre_buff.get().value_or(0);
	}

	bool is_stereo() const {
		return wav.channels > 1;
	}

	unsigned frames_available() {
		auto samples_avail = pre_buff.num_filled();
		return samples_avail / wav.channels;
	}

	bool is_eof() {
		return eof;
	}

	void seek_pos(unsigned frame_num = 0) {
		drwav_seek_to_pcm_frame(&wav, frame_num);

		// TODO: adjust read and write pos in pre_buff if frame_num is within pre_buff contents.
		// i.e. no need to read data from disk that we already have in pre_buff.
		// We would need to set the read pos to the place that contains frame_num,
		// and set the write pos to the last data following this before the seam
		//
		// For now we will just clear any prebuffered content
		pre_buff.reset();
		eof = false;
	}

private:
	drwav wav;

	bool eof = true;
	bool loaded = false;

	LockFreeFifoSpsc<float, MaxSamples> pre_buff;

	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	static constexpr unsigned ReadBlockBytes = 4096;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// e.g. 4kB of 8-bit mono will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;
	std::array<uint32_t, 4> canary{0xAAAA5555, 0x12345678, 0xBADCAFE0, 0x600DCAFE};
};

} // namespace MetaModule
