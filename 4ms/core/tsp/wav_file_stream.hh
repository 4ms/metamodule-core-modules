#pragma once
// #include "../../medium/debug_raw.h"
#include "tsp/dr_wav.h"
#include "util/lockfree_fifo_spsc.hh"
#include <cstdio>
#include <string_view>

namespace MetaModule
{

template<uint32_t MaxSamples = 1024 * 1024>
struct WavFileStream {

	bool load(std::string_view sample_path) {
		unload();

		eof = false;

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
		unsigned frames_to_read = ReadBlockBytes / wav.fmt.blockAlign;
		read_frames_from_file(frames_to_read);
	}

	void read_frames_from_file(int num_frames) {
		if (!loaded || eof)
			return;

		while (num_frames > 0) {
			// Read blocks of maximum 4kB at a time
			unsigned frames_to_read = std::min(ReadBlockBytes / wav.fmt.blockAlign, (unsigned)num_frames);

			auto frames_read = drwav_read_pcm_frames_f32(&wav, frames_to_read, read_buff.data());

			eof = (frames_read != frames_to_read);

			if (frames_read > frames_to_read) {
				printf("WavFileStream: Internal error: drwav read more frames than requested\n");
				frames_read = frames_to_read;
			}

			auto samples = std::span<const float>(read_buff.data(), frames_read * wav.channels);

			// Copy to pre-buffer, one sample at a time
			unsigned frame_ctr = 0;
			for (auto out : samples) {
				if (!pre_buff.put(out)) {
					printf("WavFileStream: Buffer overflow\n");
					// TODO: Handle buffer overflow: we read too much from disk and the audio thread
					// is not consuming the samples fast enough to make room.
					// Set drwav read cursor back to this position, pop back if we're not a frame boundary,
					// set next_frame_to_write, and abort
				}
				if (++frame_ctr >= wav.channels) {
					frame_ctr = 0;
					next_frame_to_write.store(next_frame_to_write.load() + 1);
					// printf("next_frame_to_write=%u\n", next_frame_to_write.load());
				}
			}

			// printf("requested num_frames=%d, frames_to_read=%u, frames_read=%llu. eof=%d\n",
			// 	   num_frames,
			// 	   frames_to_read,
			// 	   frames_read,
			// 	   eof);

			num_frames -= frames_read;

			if (eof) {
				// printf("EOF\n");
				break;
			}
		}
	}

	float pop_sample() {
		auto p = pre_buff.get().value_or(0);
		// printf("pop %f\n", p);
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

	unsigned current_playback_frame() const {
		return next_frame_to_write > frames_available() ? next_frame_to_write - frames_available() :
														  next_frame_to_write + total_frames() - frames_available();
	}

	unsigned total_frames() const {
		return loaded ? (unsigned)wav.totalPCMFrameCount : 0;
	}

	void reset_read_pos(unsigned frame_num = 0) {
		pre_buff.set_read_pos(frame_num * wav.channels);
	}

	void seek_frame_in_file(uint64_t frame_num = 0) {

		const auto frames_in_prebuff = std::min<unsigned>(MaxSamples / wav.channels, next_frame_to_write.load());

		// Optimization:
		// if we request to seek to a frame that's already in the prebuffer,
		// just jump the read head to there (no need to read from disk)
		if (frame_num < next_frame_to_write && (frames_in_prebuff + frame_num) >= next_frame_to_write) {
			// printf("Reset without seek: next_frame_to_write %g, frames_in_prebuff %u, frames_available %u\n",
			// 	   next_frame_to_write.load(),
			// 	   frames_in_prebuff,
			// 	   frames_available());

			pre_buff.set_read_pos(frame_num);
		} else {
			// printf(
			// 	"Reset: next_frame_to_write %f, frames_in_prebuff %u\n", next_frame_to_write.load(), frames_in_prebuff);

			// Otherwise, reset the buffer and prepare to read from disk
			drwav_seek_to_pcm_frame(&wav, frame_num);
			reset_prebuff();
			next_frame_to_write = frame_num;

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

	std::atomic<float> next_frame_to_write = 0;

	LockFreeFifoSpsc<float, MaxSamples> pre_buff;

	// assume 4kB is an efficient size to read from an SD Card or USB Drive
	static constexpr unsigned ReadBlockBytes = 4096;

	// read_buff needs to be big enough to hold 4kB of any data converted to floats
	// Worst case: 4kB of 8-bit mono data will convert to 4096 floats
	std::array<float, ReadBlockBytes> read_buff;
};

} // namespace MetaModule
