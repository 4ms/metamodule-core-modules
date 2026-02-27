#include "CoreModules/4ms/info/Djembe_info.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/moduleFactory.hh"

#include "gcem/include/gcem.hpp"
#include "util/math.hh"
#include "util/math_tables.hh"

namespace MetaModule
{

class DjembeCore : public CoreProcessor {
	using Info = DjembeInfo;
	using ThisCore = DjembeCore;

	float SAMPLERATE = 48000;

	struct alignas(16) float4 {
		float v[4];
	};

public:
	DjembeCore() {
		init_coef();

		// Todo: Combine these loops
		noise = 0;
		iRec4 = 0;
		fVecTrig = 0;

		for (int i = 0; i < 2; i++) {
			noise_hp[i] = 0.0f;
			noise_hp_lp[i] = 0.0f;
			fRec0567[i].v[0] = 0.0f;
			fRec0567[i].v[1] = 0.0f;
			fRec0567[i].v[2] = 0.0f;
			fRec0567[i].v[3] = 0.0f;
			fRec891011[i].v[0] = 0.0f;
			fRec891011[i].v[1] = 0.0f;
			fRec891011[i].v[2] = 0.0f;
			fRec891011[i].v[3] = 0.0f;
			fRec12131415[i].v[0] = 0.0f;
			fRec12131415[i].v[1] = 0.0f;
			fRec12131415[i].v[2] = 0.0f;
			fRec12131415[i].v[3] = 0.0f;
			fRec16171819[i].v[0] = 0.0f;
			fRec16171819[i].v[1] = 0.0f;
			fRec16171819[i].v[2] = 0.0f;
			fRec16171819[i].v[3] = 0.0f;
			fRec20212223[i].v[0] = 0.0f;
			fRec20212223[i].v[1] = 0.0f;
			fRec20212223[i].v[2] = 0.0f;
			fRec20212223[i].v[3] = 0.0f;
		}

		// UI
		gainCV = float(0.0f);
		gainKnob = float(1.0f);
		strikeCV = float(0.0f);
		strikeKnob = float(0.29999999999999999f);
		sharpCV = float(0.0f);
		sharpnessKnob = float(0.5f);
		trigIn = float(0.0f);
		freqCV = float(1.0f);
		freqKnob = float(60.0f);

		paramsNeedUpdating = true;
		freqNeedUpdating = true;
	}

	void update() override {
		if (bypassed) {
			signalOut = 0;
			return;
		}

		if (paramsNeedUpdating) {
			update_params();
			paramsNeedUpdating = false;
		}

		if (freqNeedUpdating) {
			calc_freq();
			freqNeedUpdating = false;
		}

		const auto slot = flipper;
		flipper ^= 1;
		const auto prev = flipper;

		// StrikeModel:
		const auto tnoise = (1103515245 * noise) + 12345;
		const auto tnoise_hp = (4.65661287e-10f * float(tnoise)) -
							   (fSlowStrike1 * ((fSlowStrike3 * noise_hp[prev]) + (fSlowStrike4 * noise_hp[slot])));

		const auto tnoise_hp_lp =
			(fSlowStrike1 *
			 (((fSlowStrike2 * tnoise_hp) + (fSlowStrike5 * noise_hp[slot])) + (fSlowStrike2 * noise_hp[prev]))) -
			(fSlowStrike6 * ((fSlowStrike7 * noise_hp_lp[prev]) + (fSlowStrike8 * noise_hp_lp[slot])));
		const auto tfVecTrig = slowTrig;
		const auto tiRec4 = ((iRec4 + (iRec4 > 0)) * (slowTrig <= fVecTrig)) + (slowTrig > fVecTrig);
		float fTemp0 = adEnvRate * float(tiRec4);
		auto adEnv = MathTools::max<float>(0.0f, MathTools::min<float>(fTemp0, (2.0f - fTemp0)));
		float noiseBurst = fSlowGainStrike * (noise_hp_lp[prev] + (tnoise_hp_lp + (2.0f * noise_hp_lp[slot]))) * adEnv;

		noise = tnoise;
		noise_hp[prev] = tnoise_hp;
		noise_hp_lp[prev] = tnoise_hp_lp;
		fVecTrig = tfVecTrig;
		iRec4 = tiRec4;

		float4 tf0567;
		float4 tf891011;
		float4 tf12131415;
		float4 tf16171819;
		float4 tf20212223;
		tf0567.v[0] =
			(noiseBurst - ((fSlow19202122.v[0] * fRec0567[slot].v[0]) + (fConst691215.v[0] * fRec0567[prev].v[0])));
		tf0567.v[1] =
			(noiseBurst - ((fSlow19202122.v[1] * fRec0567[slot].v[1]) + (fConst691215.v[1] * fRec0567[prev].v[1])));
		tf0567.v[2] =
			(noiseBurst - ((fSlow19202122.v[2] * fRec0567[slot].v[2]) + (fConst691215.v[2] * fRec0567[prev].v[2])));
		tf0567.v[3] =
			(noiseBurst - ((fSlow19202122.v[3] * fRec0567[slot].v[3]) + (fConst691215.v[3] * fRec0567[prev].v[3])));
		tf891011.v[0] = (noiseBurst - ((fSlow23242526.v[0] * fRec891011[slot].v[0]) +
									   (fConst18212427.v[0] * fRec891011[prev].v[0])));
		tf891011.v[1] = (noiseBurst - ((fSlow23242526.v[1] * fRec891011[slot].v[1]) +
									   (fConst18212427.v[1] * fRec891011[prev].v[1])));
		tf891011.v[2] = (noiseBurst - ((fSlow23242526.v[2] * fRec891011[slot].v[2]) +
									   (fConst18212427.v[2] * fRec891011[prev].v[2])));
		tf891011.v[3] = (noiseBurst - ((fSlow23242526.v[3] * fRec891011[slot].v[3]) +
									   (fConst18212427.v[3] * fRec891011[prev].v[3])));
		tf12131415.v[0] = (noiseBurst - ((fSlow27282930.v[0] * fRec12131415[slot].v[0]) +
										 (fConst30333639.v[0] * fRec12131415[prev].v[0])));
		tf12131415.v[1] = (noiseBurst - ((fSlow27282930.v[1] * fRec12131415[slot].v[1]) +
										 (fConst30333639.v[1] * fRec12131415[prev].v[1])));
		tf12131415.v[2] = (noiseBurst - ((fSlow27282930.v[2] * fRec12131415[slot].v[2]) +
										 (fConst30333639.v[2] * fRec12131415[prev].v[2])));
		tf12131415.v[3] = (noiseBurst - ((fSlow27282930.v[3] * fRec12131415[slot].v[3]) +
										 (fConst30333639.v[3] * fRec12131415[prev].v[3])));
		tf16171819.v[0] = (noiseBurst - ((fSlow31323334.v[0] * fRec16171819[slot].v[0]) +
										 (fConst42454851.v[0] * fRec16171819[prev].v[0])));
		tf16171819.v[1] = (noiseBurst - ((fSlow31323334.v[1] * fRec16171819[slot].v[1]) +
										 (fConst42454851.v[1] * fRec16171819[prev].v[1])));
		tf16171819.v[2] = (noiseBurst - ((fSlow31323334.v[2] * fRec16171819[slot].v[2]) +
										 (fConst42454851.v[2] * fRec16171819[prev].v[2])));
		tf16171819.v[3] = (noiseBurst - ((fSlow31323334.v[3] * fRec16171819[slot].v[3]) +
										 (fConst42454851.v[3] * fRec16171819[prev].v[3])));
		tf20212223.v[0] = (noiseBurst - ((fSlow35363738.v[0] * fRec20212223[slot].v[0]) +
										 (fConst54576063.v[0] * fRec20212223[prev].v[0])));
		tf20212223.v[1] = (noiseBurst - ((fSlow35363738.v[1] * fRec20212223[slot].v[1]) +
										 (fConst54576063.v[1] * fRec20212223[prev].v[1])));
		tf20212223.v[2] = (noiseBurst - ((fSlow35363738.v[2] * fRec20212223[slot].v[2]) +
										 (fConst54576063.v[2] * fRec20212223[prev].v[2])));
		tf20212223.v[3] = (noiseBurst - ((fSlow35363738.v[3] * fRec20212223[slot].v[3]) +
										 (fConst54576063.v[3] * fRec20212223[prev].v[3])));

		signalOut = 0.f;
		signalOut += 1.0f * (tf0567.v[0] - fRec0567[prev].v[0]);
		signalOut += 0.25f * (tf0567.v[1] - fRec0567[prev].v[1]);
		signalOut += 0.111111112f * (tf0567.v[2] - fRec0567[prev].v[2]);
		signalOut += 0.0625f * (tf0567.v[3] - fRec0567[prev].v[3]);
		signalOut += 0.0399999991f * (tf891011.v[0] - fRec891011[prev].v[0]);
		signalOut += 0.027777778f * (tf891011.v[1] - fRec891011[prev].v[1]);
		signalOut += 0.0204081628f * (tf891011.v[2] - fRec891011[prev].v[2]);
		signalOut += 0.015625f * (tf891011.v[3] - fRec891011[prev].v[3]);
		signalOut += 0.0123456791f * (tf12131415.v[0] - fRec12131415[prev].v[0]);
		signalOut += 0.00999999978f * (tf12131415.v[1] - fRec12131415[prev].v[1]);
		signalOut += 0.00826446246f * (tf12131415.v[2] - fRec12131415[prev].v[2]);
		signalOut += 0.0069444445f * (tf12131415.v[3] - fRec12131415[prev].v[3]);
		signalOut += 0.00591715984f * (tf16171819.v[0] - fRec16171819[prev].v[0]);
		signalOut += 0.00510204071f * (tf16171819.v[1] - fRec16171819[prev].v[1]);
		signalOut += 0.00444444455f * (tf16171819.v[2] - fRec16171819[prev].v[2]);
		signalOut += 0.00390625f * (tf16171819.v[3] - fRec16171819[prev].v[3]);
		signalOut += 0.00346020772f * (tf20212223.v[0] - fRec20212223[prev].v[0]);
		signalOut += 0.00308641978f * (tf20212223.v[1] - fRec20212223[prev].v[1]);
		signalOut += 0.00277008303f * (tf20212223.v[2] - fRec20212223[prev].v[2]);
		signalOut += 0.00249999994f * (tf20212223.v[3] - fRec20212223[prev].v[3]);

		fRec0567[prev].v[0] = tf0567.v[0];
		fRec0567[prev].v[1] = tf0567.v[1];
		fRec0567[prev].v[2] = tf0567.v[2];
		fRec0567[prev].v[3] = tf0567.v[3];
		fRec891011[prev].v[0] = tf891011.v[0];
		fRec891011[prev].v[1] = tf891011.v[1];
		fRec891011[prev].v[2] = tf891011.v[2];
		fRec891011[prev].v[3] = tf891011.v[3];
		fRec12131415[prev].v[0] = tf12131415.v[0];
		fRec12131415[prev].v[1] = tf12131415.v[1];
		fRec12131415[prev].v[2] = tf12131415.v[2];
		fRec12131415[prev].v[3] = tf12131415.v[3];
		fRec16171819[prev].v[0] = tf16171819.v[0];
		fRec16171819[prev].v[1] = tf16171819.v[1];
		fRec16171819[prev].v[2] = tf16171819.v[2];
		fRec16171819[prev].v[3] = tf16171819.v[3];
		fRec20212223[prev].v[0] = tf20212223.v[0];
		fRec20212223[prev].v[1] = tf20212223.v[1];
		fRec20212223[prev].v[2] = tf20212223.v[2];
		fRec20212223[prev].v[3] = tf20212223.v[3];
	}

	void update_params() {
		strike0 = MathTools::constrain(strikeCV + strikeKnob, 0.f, 1.f);
		strike1 = MathTools::tan_close(fConst1 * ((15000.0f * strike0) + 500.0f));
		strike2 = 1.0f / strike1;
		strike3 = ((strike2 + 1.41421354f) / strike1) + 1.0f;
		fSlowGainStrike = MathTools::min<float>(gainCV + gainKnob, 1.0f) / strike3;
		fSlow5 = MathTools::tan_close(fConst1 * ((500.0f * strike0) + 40.0f));
		fSlow6 = (1.0f / fSlow5);
		fSlowStrike1 = (1.0f / (((fSlow6 + 1.41421354f) / fSlow5) + 1.0f));
		fSlow8 = (fSlow5 * fSlow5);
		fSlowStrike2 = (1.0f / fSlow8);
		fSlowStrike3 = (((fSlow6 + -1.41421354f) / fSlow5) + 1.0f);
		fSlowStrike4 = (2.0f * (1.0f - fSlowStrike2));
		fSlowStrike5 = (0.0f - (2.0f / fSlow8));
		fSlowStrike6 = (1.0f / strike3);
		fSlowStrike7 = (((strike2 + -1.41421354f) / strike1) + 1.0f);
		fSlowStrike8 = (2.0f * (1.0f - (1.0f / (strike1 * strike1))));
		adEnvRate =
			(1.0f / MathTools::max<float>(1.0f, (fConst2 * MathTools::min<float>(sharpCV + sharpnessKnob, 1.0f))));
		slowTrig = trigIn > 0.f ? 1.f : 0.f;
	}

	void calc_freq() {
		float freq = freqCV * freqKnob * samplerateAdjust;
		slowFreq = 0.01f * freq + 0.99f * slowFreq;

		// Coef: a1
		fSlow19202122.v[0] = (fConst4 * MathTools::cos_close((fConst5 * slowFreq)));
		fSlow19202122.v[1] = (fConst8 * MathTools::cos_close((fConst5 * (slowFreq + 200.0f))));
		fSlow19202122.v[2] = (fConst11 * MathTools::cos_close((fConst5 * (slowFreq + 400.0f))));
		fSlow19202122.v[3] = (fConst14 * MathTools::cos_close((fConst5 * (slowFreq + 600.0f))));
		fSlow23242526.v[0] = (fConst17 * MathTools::cos_close((fConst5 * (slowFreq + 800.0f))));
		fSlow23242526.v[1] = (fConst20 * MathTools::cos_close((fConst5 * (slowFreq + 1000.0f))));
		fSlow23242526.v[2] = (fConst23 * MathTools::cos_close((fConst5 * (slowFreq + 1200.0f))));
		fSlow23242526.v[3] = (fConst26 * MathTools::cos_close((fConst5 * (slowFreq + 1400.0f))));
		fSlow27282930.v[0] = (fConst29 * MathTools::cos_close((fConst5 * (slowFreq + 1600.0f))));
		fSlow27282930.v[1] = (fConst32 * MathTools::cos_close((fConst5 * (slowFreq + 1800.0f))));
		fSlow27282930.v[2] = (fConst35 * MathTools::cos_close((fConst5 * (slowFreq + 2000.0f))));
		fSlow27282930.v[3] = (fConst38 * MathTools::cos_close((fConst5 * (slowFreq + 2200.0f))));
		fSlow31323334.v[0] = (fConst41 * MathTools::cos_close((fConst5 * (slowFreq + 2400.0f))));
		fSlow31323334.v[1] = (fConst44 * MathTools::cos_close((fConst5 * (slowFreq + 2600.0f))));
		fSlow31323334.v[2] = (fConst47 * MathTools::cos_close((fConst5 * (slowFreq + 2800.0f))));
		fSlow31323334.v[3] = (fConst50 * MathTools::cos_close((fConst5 * (slowFreq + 3000.0f))));
		fSlow35363738.v[0] = (fConst53 * MathTools::cos_close((fConst5 * (slowFreq + 3200.0f))));
		fSlow35363738.v[1] = (fConst56 * MathTools::cos_close((fConst5 * (slowFreq + 3400.0f))));
		fSlow35363738.v[2] = (fConst59 * MathTools::cos_close((fConst5 * (slowFreq + 3600.0f))));
		fSlow35363738.v[3] = (fConst62 * MathTools::cos_close((fConst5 * (slowFreq + 3800.0f))));
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case 0:
				freqKnob = MathTools::map_value(val, 0.f, 1.f, 20.f, 500.f);
				freqNeedUpdating = true;
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
			init_coef();
			samplerateAdjust = 48000.f / sr;
			paramsNeedUpdating = true;
			freqNeedUpdating = true;
		}
	}

	void set_input(int input_id, float v) override {
		float val = v / CvRangeVolts;

		switch (input_id) {
			case 0:
				freqCV = exp5Table.interp(MathTools::constrain(val, 0.f, 1.0f)); //1..32
				freqNeedUpdating = true;
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
				trigIn = val;
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
	bool freqNeedUpdating = false;
	float signalOut = 0;
	float samplerateAdjust = 1.f;

	float gainCV;
	float gainKnob;
	float strikeCV;
	float strikeKnob;
	float sharpCV;
	float sharpnessKnob;
	float trigIn;
	float freqCV;
	float freqKnob;
	unsigned flipper{};

	int noise{};
	float noise_hp[2]{};
	float noise_hp_lp[2]{};
	float fVecTrig{};
	int iRec4{};
	float4 fRec0567[2]{};
	float4 fRec891011[2]{};
	float4 fRec12131415[2]{};
	float4 fRec16171819[2]{};
	float4 fRec20212223[2]{};

	float strike0{};
	float strike1{};
	float strike2{};
	float strike3{};
	float fSlowGainStrike{};
	float fSlow5{};
	float fSlow6{};
	float fSlowStrike1{};
	float fSlow8{};
	float fSlowStrike2{};
	float fSlowStrike3{};
	float fSlowStrike4{};
	float fSlowStrike5{};
	float fSlowStrike6{};
	float fSlowStrike7{};
	float fSlowStrike8{};
	float adEnvRate{};
	float slowTrig{};
	float slowFreq{500.f};
	float4 fSlow19202122{};
	float4 fSlow23242526{};
	float4 fSlow27282930{};
	float4 fSlow31323334{};
	float4 fSlow35363738{};
	float fConst1{};
	float fConst2{};
	float fConst3{};
	float fConst4{};
	float fConst5{};
	float4 fConst691215{};
	float fConst7{};
	float fConst8{};
	float fConst10{};
	float fConst11{};
	float fConst13{};
	float fConst14{};
	float fConst16{};
	float fConst17{};
	float4 fConst18212427{};
	float fConst19{};
	float fConst20{};
	float fConst22{};
	float fConst23{};
	float fConst25{};
	float fConst26{};
	float fConst28{};
	float fConst29{};
	float4 fConst30333639;
	float fConst31{};
	float fConst32{};
	float fConst34{};
	float fConst35{};
	float fConst37{};
	float fConst38{};
	float fConst40{};
	float fConst41{};
	float4 fConst42454851;
	float fConst43{};
	float fConst44{};
	float fConst46{};
	float fConst47{};
	float fConst49{};
	float fConst50{};
	float fConst52{};
	float fConst53{};
	float4 fConst54576063;
	float fConst55{};
	float fConst56{};
	float fConst58{};
	float fConst59{};
	float fConst61{};
	float fConst62{};

	void init_coef() {
		fConst1 = (3.14159274f / SAMPLERATE);
		fConst2 = (0.00200000009f * SAMPLERATE);
		fConst3 = gcem::pow(0.00100000005f, (1.66666663f / SAMPLERATE));
		fConst4 = (0.0f - (2.0f * fConst3));
		fConst5 = (6.28318548f / SAMPLERATE);
		fConst691215.v[0] = (fConst3 * fConst3);
		fConst7 = gcem::pow(0.00100000005f, (1.75438595f / SAMPLERATE));
		fConst8 = (0.0f - (2.0f * fConst7));
		fConst691215.v[1] = (fConst7 * fConst7);
		fConst10 = gcem::pow(0.00100000005f, (1.85185182f / SAMPLERATE));
		fConst11 = (0.0f - (2.0f * fConst10));
		fConst691215.v[2] = (fConst10 * fConst10);
		fConst13 = gcem::pow(0.00100000005f, (1.96078432f / SAMPLERATE));
		fConst14 = (0.0f - (2.0f * fConst13));
		fConst691215.v[3] = (fConst13 * fConst13);
		fConst16 = gcem::pow(0.00100000005f, (2.08333325f / SAMPLERATE));
		fConst17 = (0.0f - (2.0f * fConst16));
		fConst18212427.v[0] = (fConst16 * fConst16);
		fConst19 = gcem::pow(0.00100000005f, (2.22222233f / SAMPLERATE));
		fConst20 = (0.0f - (2.0f * fConst19));
		fConst18212427.v[1] = (fConst19 * fConst19);
		fConst22 = gcem::pow(0.00100000005f, (2.38095236f / SAMPLERATE));
		fConst23 = (0.0f - (2.0f * fConst22));
		fConst18212427.v[2] = (fConst22 * fConst22);
		fConst25 = gcem::pow(0.00100000005f, (2.56410265f / SAMPLERATE));
		fConst26 = (0.0f - (2.0f * fConst25));
		fConst18212427.v[3] = (fConst25 * fConst25);
		fConst28 = gcem::pow(0.00100000005f, (2.77777767f / SAMPLERATE));
		fConst29 = (0.0f - (2.0f * fConst28));
		fConst30333639.v[0] = (fConst28 * fConst28);
		fConst31 = gcem::pow(0.00100000005f, (3.030303f / SAMPLERATE));
		fConst32 = (0.0f - (2.0f * fConst31));
		fConst30333639.v[1] = (fConst31 * fConst31);
		fConst34 = gcem::pow(0.00100000005f, (3.33333325f / SAMPLERATE));
		fConst35 = (0.0f - (2.0f * fConst34));
		fConst30333639.v[2] = (fConst34 * fConst34);
		fConst37 = gcem::pow(0.00100000005f, (3.70370364f / SAMPLERATE));
		fConst38 = (0.0f - (2.0f * fConst37));
		fConst30333639.v[3] = (fConst37 * fConst37);
		fConst40 = gcem::pow(0.00100000005f, (4.16666651f / SAMPLERATE));
		fConst41 = (0.0f - (2.0f * fConst40));
		fConst42454851.v[0] = (fConst40 * fConst40);
		fConst43 = gcem::pow(0.00100000005f, (4.76190472f / SAMPLERATE));
		fConst44 = (0.0f - (2.0f * fConst43));
		fConst42454851.v[1] = (fConst43 * fConst43);
		fConst46 = gcem::pow(0.00100000005f, (5.55555534f / SAMPLERATE));
		fConst47 = (0.0f - (2.0f * fConst46));
		fConst42454851.v[2] = (fConst46 * fConst46);
		fConst49 = gcem::pow(0.00100000005f, (6.66666651f / SAMPLERATE));
		fConst50 = (0.0f - (2.0f * fConst49));
		fConst42454851.v[3] = (fConst49 * fConst49);
		fConst52 = gcem::pow(0.00100000005f, (8.33333302f / SAMPLERATE));
		fConst53 = (0.0f - (2.0f * fConst52));
		fConst54576063.v[0] = (fConst52 * fConst52);
		fConst55 = gcem::pow(0.00100000005f, (11.1111107f / SAMPLERATE));
		fConst56 = (0.0f - (2.0f * fConst55));
		fConst54576063.v[1] = (fConst55 * fConst55);
		fConst58 = gcem::pow(0.00100000005f, (16.666666f / SAMPLERATE));
		fConst59 = (0.0f - (2.0f * fConst58));
		fConst54576063.v[2] = (fConst58 * fConst58);
		fConst61 = gcem::pow(0.00100000005f, (33.3333321f / SAMPLERATE));
		fConst62 = (0.0f - (2.0f * fConst61));
		fConst54576063.v[3] = (fConst61 * fConst61);
	}

	static constexpr float outputScalingVolts = 5.f;

public:
	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

	constexpr static auto indices = ElementCount::get_indices<Info>();
	constexpr static auto element_index(Info::Elem el) {
		return static_cast<std::underlying_type_t<Info::Elem>>(el);
	}

	constexpr static auto index(Info::Elem el) {
		auto element_idx = element_index(el);
		return indices[element_idx];
	}
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
