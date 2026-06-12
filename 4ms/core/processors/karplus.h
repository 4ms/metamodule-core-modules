#pragma once

#include "util/math.hh"

#ifdef CORE_CA7
#include <arm_neon.h>
#else
#include "util/soft_neon.hh"
#endif

// Polyphonic Karplus-Strong voice bank: up to 4 voices processed in parallel,
// one voice per NEON lane. All voices share the decay and spread settings, but
// each voice has its own frequency and delay-line state.
//
// Data layout: the delay buffers are voice-interleaved (buf[sample * 4 + voice]),
// so all voices share one write index and each tap's write is a single vector
// store. Reads use per-voice delay times so they must gather scalar values
// (NEON has no gather), but the feedback, mix, and DC-block arithmetic is
// vectorized across voices.
class Karplus {
public:
	static constexpr unsigned MaxVoices = 4;

	Karplus() {
		for (auto &out : lastOut)
			out = vdupq_n_f32(0.f);
		dcPrevIn = vdupq_n_f32(0.f);
		dcPrevOut = vdupq_n_f32(0.f);
	}

	// input/output: one sample per voice, 16-byte aligned float[MaxVoices].
	// Input lanes >= num_voices should be 0; their output lanes just ring out.
	void update(const float input[MaxVoices], float output[MaxVoices], unsigned num_voices) {
		const float32x4_t feedback_v = vdupq_n_f32(feedback);
		float32x4_t in_v = vld1q_f32(input);
		float32x4_t out_v = vmulq_f32(in_v, vdupq_n_f32(float(taps)));

		for (unsigned t = 0; t < taps; t++) {
			// Same topology as the original mono version: tap 0 feeds back from
			// tap 5's previous output, taps 1..5 from their own previous output.
			float32x4_t tap_in = in_v;
			tap_in = vmlaq_f32(tap_in, lastOut[t == 0 ? taps - 1 : t], feedback_v);
			vst1q_f32(&buf[t][writeIndex * MaxVoices], tap_in);

			alignas(16) float tap_out[MaxVoices]{};
			for (unsigned v = 0; v < num_voices; v++) {
				float readPos = float(writeIndex) - delaySamples[t][v];
				if (readPos < 0.f)
					readPos += float(BufSize);
				auto idx0 = static_cast<unsigned>(readPos);
				const float phase = readPos - float(idx0);
				const unsigned idx1 = (idx0 == BufSize - 1) ? 0 : idx0 + 1;
				const float d0 = buf[t][idx0 * MaxVoices + v];
				const float d1 = buf[t][idx1 * MaxVoices + v];
				tap_out[v] = d0 + (d1 - d0) * phase;
			}
			lastOut[t] = vld1q_f32(tap_out);
			out_v = vaddq_f32(out_v, lastOut[t]);
		}

		if (++writeIndex == BufSize)
			writeIndex = 0;

		out_v = vmulq_f32(out_v, vdupq_n_f32(1.f / (taps * 2.f)));

		// DC block, per voice: y[n] = x[n] - x[n-1] + 0.995 * y[n-1]
		float32x4_t dc_v = vsubq_f32(out_v, dcPrevIn);
		dc_v = vmlaq_f32(dc_v, vdupq_n_f32(0.995f), dcPrevOut);
		dcPrevIn = out_v;
		dcPrevOut = dc_v;

		vst1q_f32(output, dc_v);
	}

	void set_samplerate(float sr) {
		sampleRate = sr;
		for (unsigned v = 0; v < MaxVoices; v++)
			update_delay_samples(v);
	}

	void set_frequency(unsigned voice, float inFreq) {
		if (voice >= MaxVoices)
			return;
		basePeriod[voice] = 1.0f / inFreq;
		update_delay_samples(voice);
	}

	void set_spread(float _spread) {
		spread = MathTools::map_value(_spread, 0.0f, 1.0f, 1.001f, 1.01f);
		for (unsigned v = 0; v < MaxVoices; v++)
			update_delay_samples(v);
	}

	void set_decay(float val) {
		feedback = MathTools::map_value(val, 0.0f, 1.0f, 0.98f, 1.0f);
	}

	float get_spread() const {
		return MathTools::map_value(spread, 1.001f, 1.01f, 0.f, 1.f);
	}

	float get_decay() const {
		return MathTools::map_value(feedback, .98f, 1.f, 0.f, 1.f);
	}

private:
	static constexpr unsigned taps = 6;
	static constexpr unsigned BufSize = 2400; // max delay: 20Hz @ 48kHz

	void update_delay_samples(unsigned voice) {
		const float spreadInv = 1.0f / spread;
		float period = basePeriod[voice];
		for (unsigned t = 0; t < taps; t++) {
			delaySamples[t][voice] = MathTools::constrain(period * sampleRate, 0.f, float(BufSize));
			period *= spreadInv;
		}
	}

	alignas(16) float buf[taps][BufSize * MaxVoices]{};
	float32x4_t lastOut[taps];
	float delaySamples[taps][MaxVoices]{};
	float basePeriod[MaxVoices]{};
	unsigned writeIndex = 0;

	float sampleRate = 48000.f;
	float spread = 1.0f;
	float feedback = 0.995f;

	float32x4_t dcPrevIn;
	float32x4_t dcPrevOut;
};
