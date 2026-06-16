#pragma once

#include "util/math.hh"
#include "util/math_tables.hh"
#include <array>
#include <cstdint>
#include <memory>

// Tell the vectorizer there are no loop-carried dependencies, so it doesn't
// emit runtime overlap checks with scalar fallback loops (GCC only exploits
// __restrict on function parameters, not on local pointers).
#if defined(__clang__)
#define MM_NO_LOOP_DEPS _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define MM_NO_LOOP_DEPS _Pragma("GCC ivdep")
#else
#define MM_NO_LOOP_DEPS
#endif

// Polyphonic two-operator FM oscillator: 4 voices in struct-of-arrays layout.
// Same algorithm as TwoOpFM (twoOpFMOscillator.h), but written so the compiler
// can auto-vectorize the per-voice math with NEON (no intrinsics): all per-voice
// data is in 16-byte-aligned arrays, and every loop runs a fixed MaxVoices
// iterations. Only the sine table lookups are inherently scalar (no gather).
//
// The caller writes the per-voice control arrays (carrierFreq, modFreq,
// modAmount, shape, mix) before each update() call, and reads the result from
// the `out` array. Using aligned member arrays instead of pointer parameters
// lets the compiler vectorize without alignment/aliasing runtime checks.
class TwoOpFMPoly {
public:
	static constexpr unsigned MaxVoices = 4;

	// Per-voice controls, written by the caller:
	alignas(16) std::array<float, MaxVoices> carrierFreq{};
	alignas(16) std::array<float, MaxVoices> modFreq{};
	alignas(16) std::array<float, MaxVoices> modAmount{};
	alignas(16) std::array<float, MaxVoices> shape{};
	alignas(16) std::array<float, MaxVoices> mix{};

	// Per-voice output, written by update():
	alignas(16) std::array<float, MaxVoices> out{};

	void update(unsigned num_voices) {
		// The build uses -mno-unaligned-access, so without the alignment hints
		// the vectorizer emits runtime alignment checks with scalar fallback
		// copies of every loop ("loop versioned for vectorization"); __restrict
		// removes the equivalent runtime overlap checks.
		const float *__restrict carrierFreq_ = std::assume_aligned<16>(carrierFreq.data());
		const float *__restrict modFreq_ = std::assume_aligned<16>(modFreq.data());
		const float *__restrict modAmount_ = std::assume_aligned<16>(modAmount.data());
		const float *__restrict shape_ = std::assume_aligned<16>(shape.data());
		const float *__restrict mix_ = std::assume_aligned<16>(mix.data());
		float *__restrict out_ = std::assume_aligned<16>(out.data());
		uint32_t *__restrict phaccu0_ = std::assume_aligned<16>(phaccu0.data());
		uint32_t *__restrict phaccu1_ = std::assume_aligned<16>(phaccu1.data());
		float *__restrict sinOut1_ = std::assume_aligned<16>(sinOut1.data());

		// Phase increments. The carrier is frequency-modulated by the
		// modulator's previous output sample (same one-sample feedback as the
		// mono version). Converting a negative float to uint32_t saturates to
		// 0 on both the scalar VFP and NEON paths.
		alignas(16) std::array<uint32_t, MaxVoices> increment0;
		for (unsigned v = 0; v < MaxVoices; v++)
			increment0[v] =
				static_cast<uint32_t>((carrierFreq_[v] + sinOut1_[v] * modAmount_[v] * 4000.0f) * inv_samplerate);

		for (unsigned v = 0; v < MaxVoices; v++)
			phaccu1_[v] += static_cast<uint32_t>(modFreq_[v] * inv_samplerate);

		// Sine table lookups: scalar (NEON has no gather), so only do the active
		// voices. Inactive lanes are zero-filled (sinOut0) or keep their previous
		// value (sinOut1_); either way their out_[] is past outChans and ignored.
		alignas(16) std::array<float, MaxVoices> sinOut0{};
		for (unsigned v = 0; v < num_voices; v++)
			sinOut1_[v] = sinTable[(phaccu1_[v] + HalfMax) >> 21];
		for (unsigned v = 0; v < num_voices; v++)
			sinOut0[v] = sinTable[phaccu0_[v] >> 21];

		for (unsigned v = 0; v < MaxVoices; v++)
			phaccu0_[v] += increment0[v];

		// Square waves from the phase MSB, then shape and mix
		MM_NO_LOOP_DEPS
		for (unsigned v = 0; v < MaxVoices; v++) {
			float sqrOut0 = phaccu0_[v] > HalfMax ? 1.0f : -1.0f;
			float sqrOut1 = phaccu1_[v] > HalfMax ? 1.0f : -1.0f;
			float finalWav0 = MathTools::interpolate(sinOut0[v], sqrOut0, shape_[v]);
			float finalWav1 = MathTools::interpolate(sinOut1_[v], sqrOut1, shape_[v]);
			out_[v] = MathTools::interpolate(finalWav0, finalWav1, mix_[v]);
		}
	}

	void set_samplerate(float sr) {
		if (sr > 0.f)
			inv_samplerate = float(max_) / sr;
	}

private:
	static constexpr uint32_t max_ = 0xFFFFFFFF;
	static constexpr uint32_t HalfMax = max_ / 2;

	alignas(16) std::array<uint32_t, MaxVoices> phaccu0{};
	alignas(16) std::array<uint32_t, MaxVoices> phaccu1{};
	alignas(16) std::array<float, MaxVoices> sinOut1{};

	float inv_samplerate = float(max_) / 48000.f;
};
