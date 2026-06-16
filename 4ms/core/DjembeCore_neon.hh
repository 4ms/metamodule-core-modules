#pragma once
#include "CoreModules/4ms/info/Djembe_info.hh"
#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"

#include "gcem/include/gcem.hpp"
#include "iirneon.hh"
#include "util/math.hh"
#include "util/math_tables.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Trigger input; each voice runs the
// hand-vectorized NEON filter bank (so CPU scales with the number of active
// voices, and a mono patch costs the same as the original). The four CV
// inputs map per voice, with their highest channels feeding all upper voices.
// Strike parameters and filter coefficients are recomputed per voice only
// when an input or knob actually changes.
class DjembeCoreNeon : public PolyCoreProcessor<DjembeInfo::NumInJacks, DjembeInfo::NumOutJacks> {
	using Info = DjembeInfo;
	using ThisCore = DjembeCoreNeon;

	float SAMPLERATE = 48000;

	// Sentinel that forces the initial CV-derived calculations
	static constexpr float UninitVolts = -1e9f;

	struct Voice {
		ParallelBPIIR iirs[5];

		int32_t noise{};
		float noise_hp[2]{};
		float noise_hp_lp[2]{};
		float fVecTrig{};
		int iRec4{};
		bool flipper{};
		float slowFreq = 500.f;

		// CV-derived values, recomputed when the raw CV volts change:
		float freqCV = 1.0f;
		float gainCV = 0;
		float sharpCV = 0;
		float strikeCV = 0;
		float lastPitchV = UninitVolts;
		float lastGainV = UninitVolts;
		float lastSharpV = UninitVolts;
		float lastStrikeV = UninitVolts;
		bool paramsNeedUpdating = true;
		bool freqNeedsUpdating = true;

		// Per-voice strike model coefficients:
		float fSlowGainStrike{};
		float fSlowStrike1{};
		float fSlowStrike2{};
		float fSlowStrike3{};
		float fSlowStrike4{};
		float fSlowStrike5{};
		float fSlowStrike6{};
		float fSlowStrike7{};
		float fSlowStrike8{};
		float adEnvRate{};

		// Voice-activity gating: a struck Djembe voice always rings down to
		// silence, so once it has (and no strike is in progress) the IIR banks
		// can be skipped until the next strike. peakEnv follows the output.
		bool active = false;
		float peakEnv = 0.f;
	};

public:
	DjembeCoreNeon() {
		init_coef();

		for (auto &voice : voices) {
			for (int iir_group = 0; iir_group < 5; iir_group++) {
				float n = iir_group * 4;
				float __attribute__((aligned(16))) weights[4] = {
					1.f / ((n + 1) * (n + 1)),
					1.f / ((n + 2) * (n + 2)),
					1.f / ((n + 3) * (n + 3)),
					1.f / ((n + 4) * (n + 4)),
				};
				voice.iirs[iir_group].set_outmix(weights);
			}
		}
	}

	void update() override {
		const unsigned nv = num_voices(Info::InputTrigger_In);
		auto &out = outs[0];
		out.chans = nv;

		if (bypassed) {
			out.values = {};
			return;
		}

		auto &trig = ins[Info::InputTrigger_In];

		for (unsigned v = 0; v < nv; v++) {
			auto &vc = voices[v];

			// Per-voice CVs (highest channel feeds upward), recomputed on change.
			// Note: the CV jack and knob index mapping follows the original code
			// (index 1 adjusts gain, index 2 adjusts sharpness).
			if (float volts = ins[0].or_last(v); volts != vc.lastPitchV) {
				vc.lastPitchV = volts;
				vc.freqCV = exp5Table.interp(MathTools::constrain(volts / CvRangeVolts, 0.f, 1.0f));
				vc.freqNeedsUpdating = true;
			}
			if (float volts = ins[1].or_last(v); volts != vc.lastGainV) {
				vc.lastGainV = volts;
				vc.gainCV = volts / CvRangeVolts;
				vc.paramsNeedUpdating = true;
			}
			if (float volts = ins[2].or_last(v); volts != vc.lastSharpV) {
				vc.lastSharpV = volts;
				vc.sharpCV = volts / CvRangeVolts;
				vc.paramsNeedUpdating = true;
			}
			if (float volts = ins[3].or_last(v); volts != vc.lastStrikeV) {
				vc.lastStrikeV = volts;
				vc.strikeCV = volts / CvRangeVolts;
				vc.paramsNeedUpdating = true;
			}

			if (vc.paramsNeedUpdating) {
				update_params(vc);
				vc.paramsNeedUpdating = false;
			}
			if (vc.freqNeedsUpdating) {
				vc.freqNeedsUpdating = calc_freq(vc);
			}

			const float trigIn = trig.values[v] > 0.f ? 1.f : 0.f;

			const auto slot = vc.flipper;
			vc.flipper = !vc.flipper;
			const auto prev = vc.flipper;

			// StrikeModel:
			// signed integer overflow is OK here:
			const int32_t tnoise = (1103515245 * vc.noise) + 12345;
			const auto tnoise_hp =
				(4.65661287e-10f * float(tnoise)) -
				(vc.fSlowStrike1 * ((vc.fSlowStrike3 * vc.noise_hp[prev]) + (vc.fSlowStrike4 * vc.noise_hp[slot])));

			const auto tnoise_hp_lp =
				(vc.fSlowStrike1 * (((vc.fSlowStrike2 * tnoise_hp) + (vc.fSlowStrike5 * vc.noise_hp[slot])) +
									(vc.fSlowStrike2 * vc.noise_hp[prev]))) -
				(vc.fSlowStrike6 *
				 ((vc.fSlowStrike7 * vc.noise_hp_lp[prev]) + (vc.fSlowStrike8 * vc.noise_hp_lp[slot])));
			const auto tfVecTrig = trigIn;
			const auto tiRec4 = ((vc.iRec4 + (vc.iRec4 > 0)) * (trigIn <= vc.fVecTrig)) + (trigIn > vc.fVecTrig);
			float fTemp0 = vc.adEnvRate * float(tiRec4);
			auto adEnv = MathTools::max<float>(0.0f, MathTools::min<float>(fTemp0, (2.0f - fTemp0)));
			float noiseBurst =
				vc.fSlowGainStrike * (vc.noise_hp_lp[prev] + (tnoise_hp_lp + (2.0f * vc.noise_hp_lp[slot]))) * adEnv;

			vc.noise = tnoise;
			vc.noise_hp[prev] = tnoise_hp;
			vc.noise_hp_lp[prev] = tnoise_hp_lp;
			vc.fVecTrig = tfVecTrig;
			vc.iRec4 = tiRec4;

			// IIR banks (the bulk of the per-voice cost) only need to run while
			// the voice is excited or still ringing. A struck resonator always
			// decays, so once the strike envelope is done and the output has
			// fallen silent the banks are skipped until the next strike. The cheap
			// StrikeModel above keeps running regardless, so the noise excitation
			// stays continuous (no audible seam on the next strike).
			if (vc.active || adEnv > 0.f) {
				vc.active = true;

				// Accumulate all five 4-wide filter banks in one NEON register and
				// reduce once. Reducing each bank to a scalar separately costs five
				// NEON->ARM transfers per voice, which stall the in-order Cortex-A7.
				float32x4_t iirAcc = vc.iirs[0].calc_4iir_vec(noiseBurst);
				iirAcc = vaddq_f32(iirAcc, vc.iirs[1].calc_4iir_vec(noiseBurst));
				iirAcc = vaddq_f32(iirAcc, vc.iirs[2].calc_4iir_vec(noiseBurst));
				iirAcc = vaddq_f32(iirAcc, vc.iirs[3].calc_4iir_vec(noiseBurst));
				iirAcc = vaddq_f32(iirAcc, vc.iirs[4].calc_4iir_vec(noiseBurst));
				float32x2_t iirSum = vpadd_f32(vget_low_f32(iirAcc), vget_high_f32(iirAcc));
				float signalOut = vget_lane_f32(iirSum, 0) + vget_lane_f32(iirSum, 1);

				out.values[v] = signalOut * (outputScalingVolts / algorithmScale);

				// Follow the output envelope; retire the voice once it has rung
				// down below ~-98 dB and the strike envelope is finished.
				const float aOut = signalOut < 0.f ? -signalOut : signalOut;
				vc.peakEnv = aOut > vc.peakEnv * kEnvDecay ? aOut : vc.peakEnv * kEnvDecay;
				if (vc.peakEnv < kSilenceThresh && adEnv == 0.f)
					vc.active = false;
			} else {
				out.values[v] = 0.f;
			}
		}
	}

	void update_params(Voice &vc) {
		//if strike:
		float strike0 = MathTools::min<float>(vc.strikeCV + strikeKnob, 1.0f);
		float strike1 = MathTools::tan_close(fConst1 * ((15000.0f * strike0) + 500.0f));
		float strike2 = (1.0f / strike1);
		float strike3 = (((strike2 + 1.41421354f) / strike1) + 1.0f);

		//if gain || strike:
		vc.fSlowGainStrike = MathTools::min<float>(vc.gainCV + gainKnob, 1.0f) / strike3;

		//if strike:
		float strike4 = MathTools::tan_close(fConst1 * ((500.0f * strike0) + 40.0f));
		float strike5 = (1.0f / strike4);
		vc.fSlowStrike1 = (1.0f / (((strike5 + 1.41421354f) / strike4) + 1.0f));
		float strike6 = (strike4 * strike4);
		vc.fSlowStrike2 = (1.0f / strike6);
		vc.fSlowStrike3 = (((strike5 + -1.41421354f) / strike4) + 1.0f);
		vc.fSlowStrike4 = (2.0f * (1.0f - vc.fSlowStrike2));
		vc.fSlowStrike5 = (0.0f - (2.0f / strike6));
		vc.fSlowStrike6 = (1.0f / strike3);
		vc.fSlowStrike7 = (((strike2 + -1.41421354f) / strike1) + 1.0f);
		vc.fSlowStrike8 = (2.0f * (1.0f - (1.0f / (strike1 * strike1))));

		//if sharp:
		vc.adEnvRate =
			1.0f / MathTools::max<float>(1.0f, (fConst2 * MathTools::min<float>(vc.sharpCV + sharpnessKnob, 1.0f)));
	}

	// Returns true while still gliding toward the target pitch, false once reached.
	bool calc_freq(Voice &vc) {
		float freq = vc.freqCV * freqKnob * samplerateAdjust;
		float diff = freq - vc.slowFreq;
		float adiff = diff < 0.f ? -diff : diff;

		// Snap and finish once within ~0.1% of target (sub-cent, inaudible).
		if (adiff <= 1e-3f * freq + 1e-4f) {
			vc.slowFreq = freq;
			set_freq_coef(vc, vc.slowFreq);
			return false;
		}

		vc.slowFreq = 0.01f * freq + 0.99f * vc.slowFreq;
		set_freq_coef(vc, vc.slowFreq);
		return true;
	}

	void set_freq_coef(Voice &vc, float freq) {
		// Coef: a1
		for (int iir_group = 0; iir_group < 5; iir_group++) {
			float __attribute__((aligned(16))) slows[4];
			for (int i = 0; i < 4; i++) {
				int n = i + iir_group * 4;
				slows[i] = iir_slow_consts[n] * MathTools::cos_close((fConst5 * (freq + 200.f * n)));
			}
			vc.iirs[iir_group].set_slows(slows);
		}
	}

	void set_param(int const param_id, const float val) override {
		switch (param_id) {
			case 0:
				freqKnob = MathTools::map_value(val, 0.f, 1.f, 20.f, 500.f);
				for (auto &voice : voices)
					voice.freqNeedsUpdating = true;
				break;

			case 1:
				gainKnob = val;
				for (auto &voice : voices)
					voice.paramsNeedUpdating = true;
				break;

			case 2:
				sharpnessKnob = val;
				for (auto &voice : voices)
					voice.paramsNeedUpdating = true;
				break;

			case 3:
				strikeKnob = val;
				for (auto &voice : voices)
					voice.paramsNeedUpdating = true;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case 0:
				return MathTools::map_value(freqKnob, 20.f, 500.f, 0.f, 1.f);

			case 1:
				return gainKnob;

			case 2:
				return sharpnessKnob;

			case 3:
				return strikeKnob;
		}
		return 0;
	}

	void set_samplerate(const float sr) override {
		if (sr > 0.f && sr != SAMPLERATE) {
			SAMPLERATE = sr;
			samplerateAdjust = 48000.f / sr;
			init_coef();
			for (auto &voice : voices) {
				voice.paramsNeedUpdating = true;
				voice.freqNeedsUpdating = true;
			}
		}
	}

private:
	static constexpr unsigned NumVoices = MaxVoices;
	std::array<Voice, NumVoices> voices{};

	float samplerateAdjust = 1.f;

	float gainKnob = 1.0f;
	float strikeKnob = 0.3f;
	float sharpnessKnob = 0.5f;
	float freqKnob = 60.0f;

	float fConst1{};
	float fConst2{};
	float fConst5{};

	float __attribute__((aligned(16))) iir_slow_consts[20];
	float __attribute__((aligned(16))) iir_consts[20];

	void init_coef() {
		fConst1 = (3.14159274f / SAMPLERATE);
		fConst2 = (0.00200000009f * SAMPLERATE);
		fConst5 = (6.28318548f / SAMPLERATE);

		float fConst3 = gcem::pow(0.00100000005f, (1.66666663f / SAMPLERATE));
		iir_slow_consts[0] = (0.0f - (2.0f * fConst3));
		iir_consts[0] = (fConst3 * fConst3);
		float fConst7 = gcem::pow(0.00100000005f, (1.75438595f / SAMPLERATE));
		iir_slow_consts[1] = (0.0f - (2.0f * fConst7));
		iir_consts[1] = (fConst7 * fConst7);
		float fConst10 = gcem::pow(0.00100000005f, (1.85185182f / SAMPLERATE));
		iir_slow_consts[2] = (0.0f - (2.0f * fConst10));
		iir_consts[2] = (fConst10 * fConst10);
		float fConst13 = gcem::pow(0.00100000005f, (1.96078432f / SAMPLERATE));
		iir_slow_consts[3] = (0.0f - (2.0f * fConst13));
		iir_consts[3] = (fConst13 * fConst13);
		float fConst16 = gcem::pow(0.00100000005f, (2.08333325f / SAMPLERATE));
		iir_slow_consts[4] = (0.0f - (2.0f * fConst16));
		iir_consts[4] = (fConst16 * fConst16);
		float fConst19 = gcem::pow(0.00100000005f, (2.22222233f / SAMPLERATE));
		iir_slow_consts[5] = (0.0f - (2.0f * fConst19));
		iir_consts[5] = (fConst19 * fConst19);
		float fConst22 = gcem::pow(0.00100000005f, (2.38095236f / SAMPLERATE));
		iir_slow_consts[6] = (0.0f - (2.0f * fConst22));
		iir_consts[6] = (fConst22 * fConst22);
		float fConst25 = gcem::pow(0.00100000005f, (2.56410265f / SAMPLERATE));
		iir_slow_consts[7] = (0.0f - (2.0f * fConst25));
		iir_consts[7] = (fConst25 * fConst25);
		float fConst28 = gcem::pow(0.00100000005f, (2.77777767f / SAMPLERATE));
		iir_slow_consts[8] = (0.0f - (2.0f * fConst28));
		iir_consts[8] = (fConst28 * fConst28);
		float fConst31 = gcem::pow(0.00100000005f, (3.030303f / SAMPLERATE));
		iir_slow_consts[9] = (0.0f - (2.0f * fConst31));
		iir_consts[9] = (fConst31 * fConst31);
		float fConst34 = gcem::pow(0.00100000005f, (3.33333325f / SAMPLERATE));
		iir_slow_consts[10] = (0.0f - (2.0f * fConst34));
		iir_consts[10] = (fConst34 * fConst34);
		float fConst37 = gcem::pow(0.00100000005f, (3.70370364f / SAMPLERATE));
		iir_slow_consts[11] = (0.0f - (2.0f * fConst37));
		iir_consts[11] = (fConst37 * fConst37);
		float fConst40 = gcem::pow(0.00100000005f, (4.16666651f / SAMPLERATE));
		iir_slow_consts[12] = (0.0f - (2.0f * fConst40));
		iir_consts[12] = (fConst40 * fConst40);
		float fConst43 = gcem::pow(0.00100000005f, (4.76190472f / SAMPLERATE));
		iir_slow_consts[13] = (0.0f - (2.0f * fConst43));
		iir_consts[13] = (fConst43 * fConst43);
		float fConst46 = gcem::pow(0.00100000005f, (5.55555534f / SAMPLERATE));
		iir_slow_consts[14] = (0.0f - (2.0f * fConst46));
		iir_consts[14] = (fConst46 * fConst46);
		float fConst49 = gcem::pow(0.00100000005f, (6.66666651f / SAMPLERATE));
		iir_slow_consts[15] = (0.0f - (2.0f * fConst49));
		iir_consts[15] = (fConst49 * fConst49);
		float fConst52 = gcem::pow(0.00100000005f, (8.33333302f / SAMPLERATE));
		iir_slow_consts[16] = (0.0f - (2.0f * fConst52));
		iir_consts[16] = (fConst52 * fConst52);
		float fConst55 = gcem::pow(0.00100000005f, (11.1111107f / SAMPLERATE));
		iir_slow_consts[17] = (0.0f - (2.0f * fConst55));
		iir_consts[17] = (fConst55 * fConst55);
		float fConst58 = gcem::pow(0.00100000005f, (16.666666f / SAMPLERATE));
		iir_slow_consts[18] = (0.0f - (2.0f * fConst58));
		iir_consts[18] = (fConst58 * fConst58);
		float fConst61 = gcem::pow(0.00100000005f, (33.3333321f / SAMPLERATE));
		iir_slow_consts[19] = (0.0f - (2.0f * fConst61));
		iir_consts[19] = (fConst61 * fConst61);

		for (auto &voice : voices) {
			for (int iir_group = 0; iir_group < 5; iir_group++) {
				voice.iirs[iir_group].set_consts(&(iir_consts[iir_group * 4]));
			}
		}
	}

	static constexpr float outputScalingVolts = 5.f;
	static constexpr float algorithmScale = 8.f;

	// Voice-gating thresholds, on the pre-scale IIR sum (full scale ~8): retire a
	// voice once its output envelope falls below ~-98 dB of full scale. kEnvDecay
	// is the peak-follower release (~42 ms), slow enough to bridge the audio
	// zero-crossings of the lowest drum tones.
	static constexpr float kSilenceThresh = 1e-4f;
	static constexpr float kEnvDecay = 0.9995f;

public:
	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on
};

/*
import("stdfaust.lib");
freqknob = hslider("v: djembe/freq", 60, 20, 500, 1);
freqcv = nentry("freqcv [CV:2]", 1, 1, 32, 0.001);
freqtotal = freqknob * freqcv;

trigbutton = button("Trigger");
gate = button("gate [CV:1]");

strikecv = nentry("v: jacks/strikecv [CV:3]", 0, 0, 1, 0.01);
sharpcv = nentry("v: jacks/shaprcv [CV:4]", 0, 0, 1, 0.01);
gaincv = nentry("v: jacks/gaincv [CV:5]", 0, 0, 1, 0.01);

strikepos = hslider("v: djembe/strike", 0.3, 0, 1, 0.01) + strikecv, 1 :min;
strikesharpness = hslider("v: djembe/sharpness", 0.5, 0, 1, 0.01) + sharpcv, 1 :min;
strikegain = hslider("v: djembe/gain", 1, 0, 1, 0.01) + gaincv, 1 :min;
process = trigbutton + gate : pm.djembe(freqtotal, strikepos, strikesharpness, strikegain);
*/

} // namespace MetaModule
