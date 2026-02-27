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

public:
	DjembeCore() {
		init_coef();

		noise = 0;
		iRec4 = 0;
		fVecTrig = 0;

		for (int i = 0; (i < 2); i = (i + 1)) {
			noise_hp[i] = 0.0f;
			noise_hp_lp[i] = 0.0f;
			fRec0[i] = 0.0f;
			fRec5[i] = 0.0f;
			fRec6[i] = 0.0f;
			fRec7[i] = 0.0f;
			fRec8[i] = 0.0f;
			fRec9[i] = 0.0f;
			fRec10[i] = 0.0f;
			fRec11[i] = 0.0f;
			fRec12[i] = 0.0f;
			fRec13[i] = 0.0f;
			fRec14[i] = 0.0f;
			fRec15[i] = 0.0f;
			fRec16[i] = 0.0f;
			fRec17[i] = 0.0f;
			fRec18[i] = 0.0f;
			fRec19[i] = 0.0f;
			fRec20[i] = 0.0f;
			fRec21[i] = 0.0f;
			fRec22[i] = 0.0f;
			fRec23[i] = 0.0f;
		}

		// UI
		gainCV = 0.0f;
		gainKnob = 1.0f;
		strikeCV = 0.0f;
		strikeKnob = 0.29999999999999999f;
		sharpCV = 0.0f;
		sharpnessKnob = 0.5f;
		trigIn = 0.0f;
		freqCV = 1.0f;
		freqKnob = 60.0f;

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
		const auto tnoise_hp =
			(4.65661287e-10f * float(tnoise)) - (fSlow7 * ((fSlow10 * noise_hp[prev]) + (fSlow11 * noise_hp[slot])));

		const auto tnoise_hp_lp =
			(fSlow7 * (((fSlow9 * tnoise_hp) + (fSlow12 * noise_hp[slot])) + (fSlow9 * noise_hp[prev]))) -
			(fSlow13 * ((fSlow14 * noise_hp_lp[prev]) + (fSlow15 * noise_hp_lp[slot])));
		const auto tfVecTrig = slowTrig;
		const auto tiRec4 = ((iRec4 + (iRec4 > 0)) * (slowTrig <= fVecTrig)) + (slowTrig > fVecTrig);
		float fTemp0 = adEnvRate * float(tiRec4);
		auto adEnv = MathTools::max<float>(0.0f, MathTools::min<float>(fTemp0, (2.0f - fTemp0)));
		float noiseBurst = fSlow4 * (noise_hp_lp[prev] + (tnoise_hp_lp + (2.0f * noise_hp_lp[slot]))) * adEnv;

		noise = tnoise;
		noise_hp[prev] = tnoise_hp;
		noise_hp_lp[prev] = tnoise_hp_lp;
		fVecTrig = tfVecTrig;
		iRec4 = tiRec4;

		const auto tf0 = (noiseBurst - ((fSlow19 * fRec0[slot]) + (fConst6 * fRec0[prev])));
		const auto tf5 = (noiseBurst - ((fSlow20 * fRec5[slot]) + (fConst9 * fRec5[prev])));
		const auto tf6 = (noiseBurst - ((fSlow21 * fRec6[slot]) + (fConst12 * fRec6[prev])));
		const auto tf7 = (noiseBurst - ((fSlow22 * fRec7[slot]) + (fConst15 * fRec7[prev])));
		const auto tf8 = (noiseBurst - ((fSlow23 * fRec8[slot]) + (fConst18 * fRec8[prev])));
		const auto tf9 = (noiseBurst - ((fSlow24 * fRec9[slot]) + (fConst21 * fRec9[prev])));
		const auto tf10 = (noiseBurst - ((fSlow25 * fRec10[slot]) + (fConst24 * fRec10[prev])));
		const auto tf11 = (noiseBurst - ((fSlow26 * fRec11[slot]) + (fConst27 * fRec11[prev])));
		const auto tf12 = (noiseBurst - ((fSlow27 * fRec12[slot]) + (fConst30 * fRec12[prev])));
		const auto tf13 = (noiseBurst - ((fSlow28 * fRec13[slot]) + (fConst33 * fRec13[prev])));
		const auto tf14 = (noiseBurst - ((fSlow29 * fRec14[slot]) + (fConst36 * fRec14[prev])));
		const auto tf15 = (noiseBurst - ((fSlow30 * fRec15[slot]) + (fConst39 * fRec15[prev])));
		const auto tf16 = (noiseBurst - ((fSlow31 * fRec16[slot]) + (fConst42 * fRec16[prev])));
		const auto tf17 = (noiseBurst - ((fSlow32 * fRec17[slot]) + (fConst45 * fRec17[prev])));
		const auto tf18 = (noiseBurst - ((fSlow33 * fRec18[slot]) + (fConst48 * fRec18[prev])));
		const auto tf19 = (noiseBurst - ((fSlow34 * fRec19[slot]) + (fConst51 * fRec19[prev])));
		const auto tf20 = (noiseBurst - ((fSlow35 * fRec20[slot]) + (fConst54 * fRec20[prev])));
		const auto tf21 = (noiseBurst - ((fSlow36 * fRec21[slot]) + (fConst57 * fRec21[prev])));
		const auto tf22 = (noiseBurst - ((fSlow37 * fRec22[slot]) + (fConst60 * fRec22[prev])));
		const auto tf23 = (noiseBurst - ((fSlow38 * fRec23[slot]) + (fConst63 * fRec23[prev])));

		signalOut = 0.f;
		signalOut += 1.0f * (tf0 - fRec0[prev]);
		signalOut += 0.25f * (tf5 - fRec5[prev]);
		signalOut += 0.111111112f * (tf6 - fRec6[prev]);
		signalOut += 0.0625f * (tf7 - fRec7[prev]);
		signalOut += 0.0399999991f * (tf8 - fRec8[prev]);
		signalOut += 0.027777778f * (tf9 - fRec9[prev]);
		signalOut += 0.0204081628f * (tf10 - fRec10[prev]);
		signalOut += 0.015625f * (tf11 - fRec11[prev]);
		signalOut += 0.0123456791f * (tf12 - fRec12[prev]);
		signalOut += 0.00999999978f * (tf13 - fRec13[prev]);
		signalOut += 0.00826446246f * (tf14 - fRec14[prev]);
		signalOut += 0.0069444445f * (tf15 - fRec15[prev]);
		signalOut += 0.00591715984f * (tf16 - fRec16[prev]);
		signalOut += 0.00510204071f * (tf17 - fRec17[prev]);
		signalOut += 0.00444444455f * (tf18 - fRec18[prev]);
		signalOut += 0.00390625f * (tf19 - fRec19[prev]);
		signalOut += 0.00346020772f * (tf20 - fRec20[prev]);
		signalOut += 0.00308641978f * (tf21 - fRec21[prev]);
		signalOut += 0.00277008303f * (tf22 - fRec22[prev]);
		signalOut += 0.00249999994f * (tf23 - fRec23[prev]);

		fRec0[prev] = tf0;
		fRec5[prev] = tf5;
		fRec6[prev] = tf6;
		fRec7[prev] = tf7;
		fRec8[prev] = tf8;
		fRec9[prev] = tf9;
		fRec10[prev] = tf10;
		fRec11[prev] = tf11;
		fRec12[prev] = tf12;
		fRec13[prev] = tf13;
		fRec14[prev] = tf14;
		fRec15[prev] = tf15;
		fRec16[prev] = tf16;
		fRec17[prev] = tf17;
		fRec18[prev] = tf18;
		fRec19[prev] = tf19;
		fRec20[prev] = tf20;
		fRec21[prev] = tf21;
		fRec22[prev] = tf22;
		fRec23[prev] = tf23;
	}

	void update_params() {
		strike0 = MathTools::constrain(strikeCV + strikeKnob, 0.f, 1.f);
		strike1 = MathTools::tan_close(fConst1 * ((15000.0f * strike0) + 500.0f));
		strike2 = 1.0f / strike1;
		strike3 = ((strike2 + 1.41421354f) / strike1) + 1.0f;
		fSlow4 = MathTools::min<float>(gainCV + gainKnob, 1.0f) / strike3;
		fSlow5 = MathTools::tan_close(fConst1 * ((500.0f * strike0) + 40.0f));
		fSlow6 = (1.0f / fSlow5);
		fSlow7 = (1.0f / (((fSlow6 + 1.41421354f) / fSlow5) + 1.0f));
		fSlow8 = (fSlow5 * fSlow5);
		fSlow9 = (1.0f / fSlow8);
		fSlow10 = (((fSlow6 + -1.41421354f) / fSlow5) + 1.0f);
		fSlow11 = (2.0f * (1.0f - fSlow9));
		fSlow12 = (0.0f - (2.0f / fSlow8));
		fSlow13 = (1.0f / strike3);
		fSlow14 = (((strike2 + -1.41421354f) / strike1) + 1.0f);
		fSlow15 = (2.0f * (1.0f - (1.0f / (strike1 * strike1))));
		adEnvRate =
			(1.0f / MathTools::max<float>(1.0f, (fConst2 * MathTools::min<float>(sharpCV + sharpnessKnob, 1.0f))));
		slowTrig = trigIn > 0.f ? 1.f : 0.f;
	}

	void calc_freq() {
		float freq = freqCV * freqKnob * samplerateAdjust;
		slowFreq = 0.01f * freq + 0.99f * slowFreq;

		// Coef: a1
		fSlow19 = (fConst4 * MathTools::cos_close((fConst5 * slowFreq)));
		fSlow20 = (fConst8 * MathTools::cos_close((fConst5 * (slowFreq + 200.0f))));
		fSlow21 = (fConst11 * MathTools::cos_close((fConst5 * (slowFreq + 400.0f))));
		fSlow22 = (fConst14 * MathTools::cos_close((fConst5 * (slowFreq + 600.0f))));
		fSlow23 = (fConst17 * MathTools::cos_close((fConst5 * (slowFreq + 800.0f))));
		fSlow24 = (fConst20 * MathTools::cos_close((fConst5 * (slowFreq + 1000.0f))));
		fSlow25 = (fConst23 * MathTools::cos_close((fConst5 * (slowFreq + 1200.0f))));
		fSlow26 = (fConst26 * MathTools::cos_close((fConst5 * (slowFreq + 1400.0f))));
		fSlow27 = (fConst29 * MathTools::cos_close((fConst5 * (slowFreq + 1600.0f))));
		fSlow28 = (fConst32 * MathTools::cos_close((fConst5 * (slowFreq + 1800.0f))));
		fSlow29 = (fConst35 * MathTools::cos_close((fConst5 * (slowFreq + 2000.0f))));
		fSlow30 = (fConst38 * MathTools::cos_close((fConst5 * (slowFreq + 2200.0f))));
		fSlow31 = (fConst41 * MathTools::cos_close((fConst5 * (slowFreq + 2400.0f))));
		fSlow32 = (fConst44 * MathTools::cos_close((fConst5 * (slowFreq + 2600.0f))));
		fSlow33 = (fConst47 * MathTools::cos_close((fConst5 * (slowFreq + 2800.0f))));
		fSlow34 = (fConst50 * MathTools::cos_close((fConst5 * (slowFreq + 3000.0f))));
		fSlow35 = (fConst53 * MathTools::cos_close((fConst5 * (slowFreq + 3200.0f))));
		fSlow36 = (fConst56 * MathTools::cos_close((fConst5 * (slowFreq + 3400.0f))));
		fSlow37 = (fConst59 * MathTools::cos_close((fConst5 * (slowFreq + 3600.0f))));
		fSlow38 = (fConst62 * MathTools::cos_close((fConst5 * (slowFreq + 3800.0f))));
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
				break;

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
	float fRec0[2]{};
	float fRec5[2]{};
	float fRec6[2]{};
	float fRec7[2]{};
	float fRec8[2]{};
	float fRec9[2]{};
	float fRec10[2]{};
	float fRec11[2]{};
	float fRec12[2]{};
	float fRec13[2]{};
	float fRec14[2]{};
	float fRec15[2]{};
	float fRec16[2]{};
	float fRec17[2]{};
	float fRec18[2]{};
	float fRec19[2]{};
	float fRec20[2]{};
	float fRec21[2]{};
	float fRec22[2]{};
	float fRec23[2]{};

	float strike0{};
	float strike1{};
	float strike2{};
	float strike3{};
	float fSlow4{};
	float fSlow5{};
	float fSlow6{};
	float fSlow7{};
	float fSlow8{};
	float fSlow9{};
	float fSlow10{};
	float fSlow11{};
	float fSlow12{};
	float fSlow13{};
	float fSlow14{};
	float fSlow15{};
	float adEnvRate{};
	float slowTrig{};
	float slowFreq{500.f};
	float fSlow19{};
	float fSlow20{};
	float fSlow21{};
	float fSlow22{};
	float fSlow23{};
	float fSlow24{};
	float fSlow25{};
	float fSlow26{};
	float fSlow27{};
	float fSlow28{};
	float fSlow29{};
	float fSlow30{};
	float fSlow31{};
	float fSlow32{};
	float fSlow33{};
	float fSlow34{};
	float fSlow35{};
	float fSlow36{};
	float fSlow37{};
	float fSlow38{};
	float fConst1{};
	float fConst2{};
	float fConst3{};
	float fConst4{};
	float fConst5{};
	float fConst6{};
	float fConst7{};
	float fConst8{};
	float fConst9{};
	float fConst10{};
	float fConst11{};
	float fConst12{};
	float fConst13{};
	float fConst14{};
	float fConst15{};
	float fConst16{};
	float fConst17{};
	float fConst18{};
	float fConst19{};
	float fConst20{};
	float fConst21{};
	float fConst22{};
	float fConst23{};
	float fConst24{};
	float fConst25{};
	float fConst26{};
	float fConst27{};
	float fConst28{};
	float fConst29{};
	float fConst30{};
	float fConst31{};
	float fConst32{};
	float fConst33{};
	float fConst34{};
	float fConst35{};
	float fConst36{};
	float fConst37{};
	float fConst38{};
	float fConst39{};
	float fConst40{};
	float fConst41{};
	float fConst42{};
	float fConst43{};
	float fConst44{};
	float fConst45{};
	float fConst46{};
	float fConst47{};
	float fConst48{};
	float fConst49{};
	float fConst50{};
	float fConst51{};
	float fConst52{};
	float fConst53{};
	float fConst54{};
	float fConst55{};
	float fConst56{};
	float fConst57{};
	float fConst58{};
	float fConst59{};
	float fConst60{};
	float fConst61{};
	float fConst62{};
	float fConst63{};

	void init_coef() {
		fConst1 = (3.14159274f / SAMPLERATE);
		fConst2 = (0.00200000009f * SAMPLERATE);
		fConst3 = gcem::pow(0.00100000005f, (1.66666663f / SAMPLERATE));
		fConst4 = (0.0f - (2.0f * fConst3));
		fConst5 = (6.28318548f / SAMPLERATE);
		fConst6 = (fConst3 * fConst3);
		fConst7 = gcem::pow(0.00100000005f, (1.75438595f / SAMPLERATE));
		fConst8 = (0.0f - (2.0f * fConst7));
		fConst9 = (fConst7 * fConst7);
		fConst10 = gcem::pow(0.00100000005f, (1.85185182f / SAMPLERATE));
		fConst11 = (0.0f - (2.0f * fConst10));
		fConst12 = (fConst10 * fConst10);
		fConst13 = gcem::pow(0.00100000005f, (1.96078432f / SAMPLERATE));
		fConst14 = (0.0f - (2.0f * fConst13));
		fConst15 = (fConst13 * fConst13);
		fConst16 = gcem::pow(0.00100000005f, (2.08333325f / SAMPLERATE));
		fConst17 = (0.0f - (2.0f * fConst16));
		fConst18 = (fConst16 * fConst16);
		fConst19 = gcem::pow(0.00100000005f, (2.22222233f / SAMPLERATE));
		fConst20 = (0.0f - (2.0f * fConst19));
		fConst21 = (fConst19 * fConst19);
		fConst22 = gcem::pow(0.00100000005f, (2.38095236f / SAMPLERATE));
		fConst23 = (0.0f - (2.0f * fConst22));
		fConst24 = (fConst22 * fConst22);
		fConst25 = gcem::pow(0.00100000005f, (2.56410265f / SAMPLERATE));
		fConst26 = (0.0f - (2.0f * fConst25));
		fConst27 = (fConst25 * fConst25);
		fConst28 = gcem::pow(0.00100000005f, (2.77777767f / SAMPLERATE));
		fConst29 = (0.0f - (2.0f * fConst28));
		fConst30 = (fConst28 * fConst28);
		fConst31 = gcem::pow(0.00100000005f, (3.030303f / SAMPLERATE));
		fConst32 = (0.0f - (2.0f * fConst31));
		fConst33 = (fConst31 * fConst31);
		fConst34 = gcem::pow(0.00100000005f, (3.33333325f / SAMPLERATE));
		fConst35 = (0.0f - (2.0f * fConst34));
		fConst36 = (fConst34 * fConst34);
		fConst37 = gcem::pow(0.00100000005f, (3.70370364f / SAMPLERATE));
		fConst38 = (0.0f - (2.0f * fConst37));
		fConst39 = (fConst37 * fConst37);
		fConst40 = gcem::pow(0.00100000005f, (4.16666651f / SAMPLERATE));
		fConst41 = (0.0f - (2.0f * fConst40));
		fConst42 = (fConst40 * fConst40);
		fConst43 = gcem::pow(0.00100000005f, (4.76190472f / SAMPLERATE));
		fConst44 = (0.0f - (2.0f * fConst43));
		fConst45 = (fConst43 * fConst43);
		fConst46 = gcem::pow(0.00100000005f, (5.55555534f / SAMPLERATE));
		fConst47 = (0.0f - (2.0f * fConst46));
		fConst48 = (fConst46 * fConst46);
		fConst49 = gcem::pow(0.00100000005f, (6.66666651f / SAMPLERATE));
		fConst50 = (0.0f - (2.0f * fConst49));
		fConst51 = (fConst49 * fConst49);
		fConst52 = gcem::pow(0.00100000005f, (8.33333302f / SAMPLERATE));
		fConst53 = (0.0f - (2.0f * fConst52));
		fConst54 = (fConst52 * fConst52);
		fConst55 = gcem::pow(0.00100000005f, (11.1111107f / SAMPLERATE));
		fConst56 = (0.0f - (2.0f * fConst55));
		fConst57 = (fConst55 * fConst55);
		fConst58 = gcem::pow(0.00100000005f, (16.666666f / SAMPLERATE));
		fConst59 = (0.0f - (2.0f * fConst58));
		fConst60 = (fConst58 * fConst58);
		fConst61 = gcem::pow(0.00100000005f, (33.3333321f / SAMPLERATE));
		fConst62 = (0.0f - (2.0f * fConst61));
		fConst63 = (fConst61 * fConst61);
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
