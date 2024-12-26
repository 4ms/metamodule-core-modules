#pragma once
#include "CoreModules/4ms/info/Djembe_info.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"

#include "gcem/include/gcem.hpp"
#include "iirneon.hh"
#include "util/math.hh"
#include "util/math_tables.hh"

namespace MetaModule
{

class DjembeCoreNeon : public CoreProcessor {
	using Info = DjembeInfo;
	using ThisCore = DjembeCoreNeon;

	float SAMPLERATE = 48000;

public:
	DjembeCoreNeon() {

		init_coef();

		noise = 0;
		fVecTrig = 0;
		iRec4 = 0;

		for (int i = 0; i < 2; i++) {
			noise_hp[i] = 0.0f;	   //3
			noise_hp_lp[i] = 0.0f; //3
		}

		for (int iir_group = 0; iir_group < 5; iir_group++) {
			iirs[iir_group].set_consts(&(iir_consts[iir_group * 4]));
		}

		for (int iir_group = 0; iir_group < 5; iir_group++) {
			float n = iir_group * 4;
			float __attribute__((aligned(16))) weights[4] = {
				1.f / ((n + 1) * (n + 1)),
				1.f / ((n + 2) * (n + 2)),
				1.f / ((n + 3) * (n + 3)),
				1.f / ((n + 4) * (n + 4)),
			};
			iirs[iir_group].set_outmix(weights);
		}

		// UI
		gainCV = 0.0f;
		gainKnob = 1.0f;
		strikeCV = 0.0f;
		strikeKnob = float(0.29999999999999999f);
		sharpCV = 0.0f;
		sharpnessKnob = 0.5f;
		trigIn = 0.0f;
		freqCV = 1.0f;
		freqKnob = float(60.0f);

		paramsNeedUpdating = true;
		freqNeedsUpdating = true;
	}

	void update() override {
		//1038us
		if (paramsNeedUpdating) {
			update_params();
			paramsNeedUpdating = false;
		}
		if (freqNeedsUpdating) {
			calc_freq();
			freqNeedsUpdating = false;
		}

		const auto slot = flipper;
		flipper = !flipper;
		const auto prev = flipper;

		// 90ns:
		// StrikeModel:
		const auto tnoise = (1103515245 * noise) + 12345;
		const auto tnoise_hp = (4.65661287e-10f * float(tnoise)) -
							   (fSlowStrike1 * ((fSlowStrike3 * noise_hp[prev]) + (fSlowStrike4 * noise_hp[slot])));

		const auto tnoise_hp_lp =
			(fSlowStrike1 *
			 (((fSlowStrike2 * tnoise_hp) + (fSlowStrike5 * noise_hp[slot])) + (fSlowStrike2 * noise_hp[prev]))) -
			(fSlowStrike6 * ((fSlowStrike7 * noise_hp_lp[prev]) + (fSlowStrike8 * noise_hp_lp[slot])));
		const auto tfVecTrig = trigIn;
		const auto tiRec4 = ((iRec4 + (iRec4 > 0)) * (trigIn <= fVecTrig)) + (trigIn > fVecTrig);
		float fTemp0 = adEnvRate * float(tiRec4);
		auto adEnv = MathTools::max<float>(0.0f, MathTools::min<float>(fTemp0, (2.0f - fTemp0)));
		float noiseBurst = fSlowGainStrike * (noise_hp_lp[prev] + (tnoise_hp_lp + (2.0f * noise_hp_lp[slot]))) * adEnv;

		noise = tnoise;
		noise_hp[prev] = tnoise_hp;
		noise_hp_lp[prev] = tnoise_hp_lp;
		fVecTrig = tfVecTrig;
		iRec4 = tiRec4;
		//IIRs:
		//440ns vs. 500ns non-neon = 10% faster
		// Debug::Pin1::high();
		signalOut = 0.f;
		signalOut += iirs[0].calc_4iir(noiseBurst);
		signalOut += iirs[1].calc_4iir(noiseBurst);
		signalOut += iirs[2].calc_4iir(noiseBurst);
		signalOut += iirs[3].calc_4iir(noiseBurst);
		signalOut += iirs[4].calc_4iir(noiseBurst);
		// Debug::Pin1::low();
	}

	void update_params() {
		//460ns to set_freq_coef
		//if strike:
		float strike0 = MathTools::min<float>(strikeCV + strikeKnob, 1.0f);
		float strike1 = MathTools::tan_close(fConst1 * ((15000.0f * strike0) + 500.0f));
		float strike2 = (1.0f / strike1);
		float strike3 = (((strike2 + 1.41421354f) / strike1) + 1.0f);

		//if gain || strike:
		fSlowGainStrike = MathTools::min<float>(gainCV + gainKnob, 1.0f) / strike3;

		//if strike:
		float strike4 = MathTools::tan_close(fConst1 * ((500.0f * strike0) + 40.0f));
		float strike5 = (1.0f / strike4);
		fSlowStrike1 = (1.0f / (((strike5 + 1.41421354f) / strike4) + 1.0f));
		float strike6 = (strike4 * strike4);
		fSlowStrike2 = (1.0f / strike6);
		fSlowStrike3 = (((strike5 + -1.41421354f) / strike4) + 1.0f);
		fSlowStrike4 = (2.0f * (1.0f - fSlowStrike2));
		fSlowStrike5 = (0.0f - (2.0f / strike6));
		fSlowStrike6 = (1.0f / strike3);
		fSlowStrike7 = (((strike2 + -1.41421354f) / strike1) + 1.0f);
		fSlowStrike8 = (2.0f * (1.0f - (1.0f / (strike1 * strike1))));

		//if sharp:
		adEnvRate =
			1.0f / MathTools::max<float>(1.0f, (fConst2 * MathTools::min<float>(sharpCV + sharpnessKnob, 1.0f)));

		//640ns
		// if freq
		// set_freq_coef(freqCV * freqKnob);
	}

	void calc_freq() {
		float freq = freqCV * freqKnob * samplerateAdjust;
		slowFreq = 0.01f * freq + 0.99f * slowFreq;

		set_freq_coef(slowFreq);
	}

	void set_freq_coef(float freq) {
		// Coef: a1
		for (int iir_group = 0; iir_group < 5; iir_group++) {
			float __attribute__((aligned(16))) slows[4];
			for (int i = 0; i < 4; i++) {
				int n = i + iir_group * 4;
				slows[i] = iir_slow_consts[n] * MathTools::cos_close((fConst5 * (freq + 200.f * n)));
			}
			iirs[iir_group].set_slows(slows);
		}
	}

	void set_param(int const param_id, const float val) override {
		switch (param_id) {
			case 0:
				freqKnob = MathTools::map_value(val, 0.f, 1.f, 20.f, 500.f);
				freqNeedsUpdating = true;
				break;

			case 1:
				gainKnob = val;
				paramsNeedUpdating = true;
				break;

			case 2:
				sharpnessKnob = val;
				paramsNeedUpdating = true;
				break;

			case 3:
				strikeKnob = val;
				paramsNeedUpdating = true;
				break;
		}
	}

	void set_samplerate(const float sr) override {
		if (sr > 0.f && sr != SAMPLERATE) {
			SAMPLERATE = sr;
			samplerateAdjust = 48000.f / sr;
			init_coef();
			paramsNeedUpdating = true;
			freqNeedsUpdating = true;
		}
	}

	void set_input(const int input_id, const float v) override {
		float val = v / CvRangeVolts;

		switch (input_id) {
			case 0:
				freqCV = exp5Table.interp(MathTools::constrain(val, 0.f, 1.0f));
				freqNeedsUpdating = true;
				break;

			case 1:
				gainCV = val;
				paramsNeedUpdating = true;
				break;

			case 2:
				sharpCV = val;
				paramsNeedUpdating = true;
				break;

			case 3:
				strikeCV = val;
				paramsNeedUpdating = true;
				break;

			case 4:
				trigIn = val > 0.f ? 1.f : 0.f;
				paramsNeedUpdating = true;
				break;
		}
	}

	float get_output(const int output_id) const override {
		constexpr float algorithmScale = 8.f;
		return signalOut * (outputScalingVolts / algorithmScale);
	}

private:
	bool paramsNeedUpdating = false;
	bool freqNeedsUpdating = false;
	float signalOut = 0;
	float samplerateAdjust = 1.f;
	float slowFreq = 500.f;

	ParallelBPIIR iirs[5];

	float gainCV;
	float gainKnob;
	float strikeCV;
	float strikeKnob;
	int noise{};
	float noise_hp[2]{};
	float noise_hp_lp[2]{};
	float sharpCV;
	float sharpnessKnob;
	float trigIn;
	float fVecTrig{};
	int iRec4{};
	float freqCV;
	float freqKnob;
	bool flipper{};

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

		for (int iir_group = 0; iir_group < 5; iir_group++) {
			iirs[iir_group].set_consts(&(iir_consts[iir_group * 4]));
		}

		float freq = freqCV * freqKnob * samplerateAdjust;
		set_freq_coef(freq);
	}

	static constexpr float outputScalingVolts = 5.f;

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
