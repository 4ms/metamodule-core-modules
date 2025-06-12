#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Drum_info.hh"
#include "processors/envelope.h"
#include "processors/twoOpFMOscillator.h"

namespace MetaModule
{

class DrumCore : public CoreProcessor {
	using Info = DrumInfo;
	using ThisCore = DrumCore;

private:
	enum { pitchEnvelope, fmEnvelope, toneEnvelope, noiseEnvelope };

	Envelope envelopes[4];
	TwoOpFM osc;

	float gateIn = 0;
	float drumOutput = 0;
	float baseFrequency = 50;
	float noiseBlend = 0.5f;
	float pitchAmount = 0;
	float fmAmount = 0;
	float pitchCV = 20;
	bool pitchConnected = false;

	float baseNoiseEnvTime = 0;
	float baseToneEnvTime = 0;
	float basePitchEnvTime = 0;
	float baseFMEnvTime = 0;

	float noiseEnvCV = 0;
	float toneEnvCV = 0;
	float pitchEnvCV = 0;
	float FMEnvCV = 0;
	float ratioCV = 0;
	float fmAmountCV = 0;
	float pitchAmountCV = 0;
	float noiseBlendCV = 0;

	float ratio = 1;

	float tenv;

	InterpArray<float, 4> pitchDecayTimes = {10, 10, 200, 500};
	InterpArray<float, 4> pitchBreakPoint = {0, 0.1, 0.2, 1};
	InterpArray<float, 4> pitchReleaseTimes = {50, 300, 500, 3000};

	InterpArray<float, 7> toneAttackTimes = {1, 1, 3, 5, 7, 9, 20};
	InterpArray<float, 6> toneHoldTimes = {0, 20, 50, 70, 100, 600};
	InterpArray<float, 3> toneDecayTimes = {10, 200, 600};
	InterpArray<float, 4> toneBreakPoint = {0.1, 0.2, 0.8};
	InterpArray<float, 3> toneReleaseTimes = {10, 200, 500};

public:
	DrumCore() {
		for (int i = 0; i < 4; i++) {
			envelopes[i].sustainEnable = false;
			envelopes[i].set_attack_curve(0);
			envelopes[i].set_decay_curve(0);
			envelopes[i].set_release_curve(0.5f);
			envelopes[i].set_sustain(0.2f);
		}

		envelopes[toneEnvelope].set_envelope_time(0, 1);
		envelopes[toneEnvelope].set_envelope_time(1, 50);
		envelopes[toneEnvelope].set_envelope_time(2, 100);
		envelopes[toneEnvelope].set_envelope_time(4, 2000);
		envelopes[toneEnvelope].set_release_curve(1.0f);

		envelopes[fmEnvelope].set_envelope_time(0, 1);
		envelopes[fmEnvelope].set_envelope_time(1, 0);
		envelopes[fmEnvelope].set_envelope_time(2, 300);
		envelopes[fmEnvelope].set_envelope_time(4, 700);

		envelopes[noiseEnvelope].set_envelope_time(0, 1);
		envelopes[noiseEnvelope].set_envelope_time(1, 0);
		envelopes[noiseEnvelope].set_envelope_time(2, 30);
		envelopes[noiseEnvelope].set_envelope_time(4, 700);
		envelopes[noiseEnvelope].set_release_curve(0.0f);

		envelopes[pitchEnvelope].set_envelope_time(0, 5.0);
		envelopes[pitchEnvelope].set_envelope_time(1, 0);
		envelopes[pitchEnvelope].set_envelope_time(2, 50);
		envelopes[pitchEnvelope].set_envelope_time(4, 2000);

		setToneEnvelope();
		setFMEnvelope();
		setNoiseEnvelope();
		setPitchEnvelope();
	}

	void update() override {
		float pitchAmt = std::clamp(pitchAmount + pitchAmountCV, 0.f, 1.f);
		auto freqCalc = baseFrequency + (envelopes[pitchEnvelope].update(gateIn) * 4000.0f * (pitchAmt * pitchAmt));

		float ratioTot = MathTools::map_value(std::clamp(ratio + ratioCV, 0.f, 1.f), 0.0f, 1.0f, 1.0f, 16.0f);
		osc.set_frequency(1, baseFrequency * ratioTot);

		if (pitchConnected) {
			osc.set_frequency(0, freqCalc * MathTools::setPitchMultiple(pitchCV));
		} else {
			osc.set_frequency(0, freqCalc);
		}

		osc.modAmount = envelopes[fmEnvelope].update(gateIn) * std::clamp(fmAmount + fmAmountCV, 0.f, 1.f);
		auto noiseOut = MathTools::randomNumber(-1.0f, 1.0f) * envelopes[noiseEnvelope].update(gateIn);

		tenv = envelopes[toneEnvelope].update(gateIn);
		auto toneOutput = osc.update() * tenv;

		auto noiseBlendTot = std::clamp(noiseBlend + noiseBlendCV, 0.f, 1.f);
		drumOutput = MathTools::interpolate(toneOutput, noiseOut, noiseBlendTot);
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobPitch:
				baseFrequency = MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 1000.0f);
				break;
			case Info::KnobPitch_Env: // pitch envelope
				basePitchEnvTime = val;
				setPitchEnvelope();
				break;
			case Info::KnobPitch_Amount:
				pitchAmount = val;
				break;
			case Info::KnobFm_Ratio:
				ratio = val;
				break;
			case Info::KnobFm_Env: // fm envelope
				baseFMEnvTime = val;
				setFMEnvelope();
				break;
			case Info::KnobFm_Amount:
				fmAmount = val;
				break;
			case Info::KnobTone_Env: // tone envelope
				baseToneEnvTime = val;
				setToneEnvelope();
				break;
			case Info::KnobNoise_Env: // noise envelope
				baseNoiseEnvTime = val;
				setNoiseEnvelope();
				break;
			case Info::KnobNoise_Blend:
				noiseBlend = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobPitch:
				return MathTools::map_value(baseFrequency, 10.0f, 1000.0f, 0.f, 1.f);
			case Info::KnobPitch_Env: // pitch envelope
				return basePitchEnvTime;
			case Info::KnobPitch_Amount:
				return pitchAmount;
			case Info::KnobFm_Ratio:
				return ratio;
			case Info::KnobFm_Env: // fm envelope
				return baseFMEnvTime;
			case Info::KnobFm_Amount:
				return fmAmount;
			case Info::KnobTone_Env: // tone envelope
				return baseToneEnvTime;
			case Info::KnobNoise_Env: // noise envelope
				return baseNoiseEnvTime;
			case Info::KnobNoise_Blend:
				return noiseBlend;
		}
		return 0;
	}

	void setFMEnvelope() {
		float val = MathTools::constrain(baseFMEnvTime + FMEnvCV, 0.0f, 1.0f);
		envelopes[fmEnvelope].set_envelope_time(Envelope::ATTACK, MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 100.0f));
		envelopes[fmEnvelope].set_envelope_time(Envelope::DECAY, MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 8000.0f));
		envelopes[fmEnvelope].set_envelope_time(Envelope::RELEASE,
												MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 3000.0f));
		envelopes[fmEnvelope].set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.3f));
	}

	void setToneEnvelope() {
		float val = MathTools::constrain(baseToneEnvTime + toneEnvCV, 0.0f, 1.0f);
		envelopes[toneEnvelope].set_envelope_time(Envelope::ATTACK, toneAttackTimes.interp(val));
		envelopes[toneEnvelope].set_envelope_time(Envelope::HOLD, toneHoldTimes.interp(val));
		envelopes[toneEnvelope].set_envelope_time(Envelope::DECAY, toneDecayTimes.interp(val));
		envelopes[toneEnvelope].set_envelope_time(Envelope::RELEASE, toneReleaseTimes.interp(val));
		envelopes[toneEnvelope].set_sustain(toneBreakPoint.interp(val));
	}

	void setNoiseEnvelope() {
		float val = MathTools::constrain(baseNoiseEnvTime + noiseEnvCV, 0.0f, 1.0f);
		envelopes[noiseEnvelope].set_envelope_time(Envelope::ATTACK,
												   MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 50.0f));
		envelopes[noiseEnvelope].set_envelope_time(Envelope::DECAY,
												   MathTools::map_value(val, 0.0f, 1.0f, 30.0f, 100.0f));
		envelopes[noiseEnvelope].set_envelope_time(Envelope::RELEASE,
												   MathTools::map_value(val, 0.0f, 1.0f, 100.0f, 3000.0f));
		envelopes[noiseEnvelope].set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.25f));
	}

	void setPitchEnvelope() {
		float val = MathTools::constrain(pitchEnvCV + basePitchEnvTime, 0.0f, 1.0f);
		envelopes[pitchEnvelope].set_envelope_time(Envelope::DECAY, pitchDecayTimes.interp(val));
		envelopes[pitchEnvelope].set_envelope_time(Envelope::RELEASE, pitchReleaseTimes.interp(val));
		envelopes[pitchEnvelope].set_sustain(pitchBreakPoint.interp(val));
	}

	void set_samplerate(float sr) override {
		for (int i = 0; i < 4; i++) {
			envelopes[i].set_samplerate(sr);
		}
		osc.set_samplerate(sr);
	}

	void set_input(int input_id, float val) override {
		val = val / CvRangeVolts;

		switch (input_id) {
			case Info::InputTrigger_In:
				gateIn = val;
				break;
			case Info::InputV_Oct_In:
				pitchCV = val;
				break;
			case Info::InputNoise_Env_Cv_In:
				noiseEnvCV = val;
				setNoiseEnvelope();
				break;
			case Info::InputFm_Env_Cv_In:
				FMEnvCV = val;
				setFMEnvelope();
				break;
			case Info::InputPitch_Env_Cv_In:
				pitchEnvCV = val;
				setPitchEnvelope();
				break;
			case Info::InputTone_Env_Cv_In:
				toneEnvCV = val;
				setToneEnvelope();
				break;
			case Info::InputPitch_Amount_Cv_In:
				pitchAmountCV = val;
				break;
			case Info::InputNoise_Blend_Cv_In:
				noiseBlendCV = val;
				break;
			case Info::InputFm_Amount_Cv_In:
				fmAmountCV = val;
				break;
			case Info::InputRatio_Cv_In:
				ratioCV = val;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return drumOutput * outputVolts;
		if (output_id == Info::OutputTone_Env__Out)
			return tenv * outputVolts;
		return 0.f;
	}

	void mark_input_unpatched(int input_id) override {
		if (input_id == Info::InputV_Oct_In) {
			pitchConnected = false;
		}
	}
	void mark_input_patched(int input_id) override {
		if (input_id == Info::InputV_Oct_In) {
			pitchConnected = true;
		}
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

	static constexpr float outputVolts = 5.f;
};

} // namespace MetaModule
