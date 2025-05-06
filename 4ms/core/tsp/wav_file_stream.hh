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
		printf("drwav_init => %d\n", ok);
		return ok;
	}

	void buffer_frames(unsigned num_frames) {
		if (eof)
			return;

		while (num_frames > 0) {

			auto frames_to_read = std::min(ReadBlockSize / wav.channels, num_frames);

			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

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

	float get_sample() {
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

private:
	drwav wav;

	bool eof = false;

	LockFreeFifoSpsc<float, MaxFrames> pre_buff;

	static constexpr unsigned ReadBlockSize = 4096 / 4;

	std::array<float, ReadBlockSize> read_buff;
};

} // namespace MetaModule
