#pragma once
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<size_t MaxFrames = 1024 * 1024>
struct WavFileStream {

	bool init(std::string_view sample_path) {
		pre_buff.reset();
		eof = false;

		// TODO: if we already have this file loaded, then just reset the read position
		bool ok = drwav_init_file(&wav, sample_path.data(), nullptr);
		return ok;
	}

	void push_frames_from_file(unsigned num_frames) {
		if (eof)
			return;

		while (num_frames > 0) {

			auto frames_to_read = std::min(ReadBlockSize / wav.channels, num_frames);

			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());
			printf("Did read %u + %u/%u\n", wav.readCursorInPCMFrames, frames_to_read, frames_read);

			eof = (frames_read != frames_to_read);

			for (auto sample : std::span<float>(read_buff.begin(), frames_read * wav.channels)) {
				if (!pre_buff.put(sample))
					break; //buffer overflow
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

	bool eof = false;

	LockFreeFifoSpsc<float, MaxFrames> pre_buff;

	static constexpr unsigned ReadBlockSize = 4096 / 4;

	std::array<float, ReadBlockSize> read_buff;
};

} // namespace MetaModule
