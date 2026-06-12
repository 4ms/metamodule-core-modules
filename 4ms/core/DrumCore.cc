#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Drum_info.hh"
#include "processors/envelope.h"
#include "processors/twoOpFMOscillator.h"

namespace MetaModule
{

// Polyphonic: voice count follows the Trigger In jack; each voice has its own
// four envelopes and FM oscillator. The CV inputs (including V/Oct) map per
// voice, with their highest channels feeding all upper voices.
class DrumCore : public PolyCoreProcessor<DrumInfo::NumInJacks, DrumInfo::NumOutJacks> {
	using Info = DrumInfo;
	using ThisCore = DrumCore;

private:
	enum { pitchEnvelope, fmEnvelope, toneEnvelope, noiseEnvelope };

	struct Voice {
		Envelope envelopes[4];
		TwoOpFM osc;

		// Per-voice CV values (highest CV channel feeds upward):
		float noiseEnvCV = 0;
		float toneEnvCV = 0;
		float pitchEnvCV = 0;
		float FMEnvCV = 0;
	};
	std::array<Voice, MaxVoices> voices;

	float baseFrequency = 50;
	float noiseBlend = 0.5f;
	float pitchAmount = 0;
	float fmAmount = 0;

	float baseNoiseEnvTime = 0;
	float baseToneEnvTime = 0;
	float basePitchEnvTime = 0;
	float baseFMEnvTime = 0;

	float ratio = 1;

	std::array<uint32_t, MaxVoices> noiseState{0x12345678u, 0x9abcdef0u, 0xfedcba98u, 0x76543210u};

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
		for (auto &voice : voices) {
			for (int i = 0; i < 4; i++) {
				voice.envelopes[i].sustainEnable = false;
				voice.envelopes[i].set_attack_curve(0);
				voice.envelopes[i].set_decay_curve(0);
				voice.envelopes[i].set_release_curve(0.5f);
				voice.envelopes[i].set_sustain(0.2f);
			}

			voice.envelopes[toneEnvelope].set_envelope_time(0, 1);
			voice.envelopes[toneEnvelope].set_envelope_time(1, 50);
			voice.envelopes[toneEnvelope].set_envelope_time(2, 100);
			voice.envelopes[toneEnvelope].set_envelope_time(4, 2000);
			voice.envelopes[toneEnvelope].set_release_curve(1.0f);

			voice.envelopes[fmEnvelope].set_envelope_time(0, 1);
			voice.envelopes[fmEnvelope].set_envelope_time(1, 0);
			voice.envelopes[fmEnvelope].set_envelope_time(2, 300);
			voice.envelopes[fmEnvelope].set_envelope_time(4, 700);

			voice.envelopes[noiseEnvelope].set_envelope_time(0, 1);
			voice.envelopes[noiseEnvelope].set_envelope_time(1, 0);
			voice.envelopes[noiseEnvelope].set_envelope_time(2, 30);
			voice.envelopes[noiseEnvelope].set_envelope_time(4, 700);
			voice.envelopes[noiseEnvelope].set_release_curve(0.0f);

			voice.envelopes[pitchEnvelope].set_envelope_time(0, 5.0);
			voice.envelopes[pitchEnvelope].set_envelope_time(1, 0);
			voice.envelopes[pitchEnvelope].set_envelope_time(2, 50);
			voice.envelopes[pitchEnvelope].set_envelope_time(4, 2000);
		}

		for (unsigned v = 0; v < MaxVoices; v++) {
			setToneEnvelope(v);
			setFMEnvelope(v);
			setNoiseEnvelope(v);
			setPitchEnvelope(v);
		}
	}

	void update() override {
		const unsigned nv = num_voices(Info::InputTrigger_In);
		auto &audioOut = outs[Info::OutputAudio_Out];
		auto &toneEnvOut = outs[Info::OutputTone_Env__Out];
		audioOut.chans = nv;
		toneEnvOut.chans = nv;

		if (bypassed) {
			audioOut.values = {};
			toneEnvOut.values = {};
			return;
		}

		auto &trig = ins[Info::InputTrigger_In];
		auto &voct = ins[Info::InputV_Oct_In];
		const bool pitchConnected = voct.is_patched();

		// Envelope shapes use table lookups: only recompute on CV changes
		for (unsigned v = 0; v < nv; v++) {
			update_env_cv(v, &Voice::noiseEnvCV, ins[Info::InputNoise_Env_Cv_In], &ThisCore::setNoiseEnvelope);
			update_env_cv(v, &Voice::FMEnvCV, ins[Info::InputFm_Env_Cv_In], &ThisCore::setFMEnvelope);
			update_env_cv(v, &Voice::pitchEnvCV, ins[Info::InputPitch_Env_Cv_In], &ThisCore::setPitchEnvelope);
			update_env_cv(v, &Voice::toneEnvCV, ins[Info::InputTone_Env_Cv_In], &ThisCore::setToneEnvelope);
		}

		for (unsigned v = 0; v < nv; v++) {
			auto &voice = voices[v];
			float gateIn = trig.values[v] / CvRangeVolts;

			float pitchAmountCV = ins[Info::InputPitch_Amount_Cv_In].or_last(v) / CvRangeVolts;
			float pitchAmt = std::clamp(pitchAmount + pitchAmountCV, 0.f, 1.f);
			auto freqCalc =
				baseFrequency + (voice.envelopes[pitchEnvelope].update(gateIn) * 4000.0f * (pitchAmt * pitchAmt));

			float ratioCV = ins[Info::InputRatio_Cv_In].or_last(v) / CvRangeVolts;
			float ratioTot = MathTools::map_value(std::clamp(ratio + ratioCV, 0.f, 1.f), 0.0f, 1.0f, 1.0f, 16.0f);
			voice.osc.set_frequency(1, baseFrequency * ratioTot);

			if (pitchConnected) {
				float pitchCV = voct.or_last(v) / CvRangeVolts;
				voice.osc.set_frequency(0, freqCalc * MathTools::setPitchMultiple(pitchCV));
			} else {
				voice.osc.set_frequency(0, freqCalc);
			}

			float fmAmountCV = ins[Info::InputFm_Amount_Cv_In].or_last(v) / CvRangeVolts;
			voice.osc.modAmount = voice.envelopes[fmEnvelope].update(gateIn) * std::clamp(fmAmount + fmAmountCV, 0.f, 1.f);
			auto noiseOut = noise(v) * voice.envelopes[noiseEnvelope].update(gateIn);

			float tenv = voice.envelopes[toneEnvelope].update(gateIn);
			auto toneOutput = voice.osc.update() * tenv;

			float noiseBlendCV = ins[Info::InputNoise_Blend_Cv_In].or_last(v) / CvRangeVolts;
			auto noiseBlendTot = std::clamp(noiseBlend + noiseBlendCV, 0.f, 1.f);
			audioOut.values[v] = MathTools::interpolate(toneOutput, noiseOut, noiseBlendTot) * outputVolts;
			toneEnvOut.values[v] = tenv * outputVolts;
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobPitch:
				baseFrequency = MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 1000.0f);
				break;
			case Info::KnobPitch_Env: // pitch envelope
				basePitchEnvTime = val;
				for (unsigned v = 0; v < MaxVoices; v++)
					setPitchEnvelope(v);
				break;
			case Info::KnobPitch_Amount:
				pitchAmount = val;
				break;
			case Info::KnobFm_Ratio:
				ratio = val;
				break;
			case Info::KnobFm_Env: // fm envelope
				baseFMEnvTime = val;
				for (unsigned v = 0; v < MaxVoices; v++)
					setFMEnvelope(v);
				break;
			case Info::KnobFm_Amount:
				fmAmount = val;
				break;
			case Info::KnobTone_Env: // tone envelope
				baseToneEnvTime = val;
				for (unsigned v = 0; v < MaxVoices; v++)
					setToneEnvelope(v);
				break;
			case Info::KnobNoise_Env: // noise envelope
				baseNoiseEnvTime = val;
				for (unsigned v = 0; v < MaxVoices; v++)
					setNoiseEnvelope(v);
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

	void setFMEnvelope(unsigned v) {
		float val = MathTools::constrain(baseFMEnvTime + voices[v].FMEnvCV, 0.0f, 1.0f);
		auto &env = voices[v].envelopes[fmEnvelope];
		env.set_envelope_time(Envelope::ATTACK, MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 100.0f));
		env.set_envelope_time(Envelope::DECAY, MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 8000.0f));
		env.set_envelope_time(Envelope::RELEASE, MathTools::map_value(val, 0.0f, 1.0f, 10.0f, 3000.0f));
		env.set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.3f));
	}

	void setToneEnvelope(unsigned v) {
		float val = MathTools::constrain(baseToneEnvTime + voices[v].toneEnvCV, 0.0f, 1.0f);
		auto &env = voices[v].envelopes[toneEnvelope];
		env.set_envelope_time(Envelope::ATTACK, toneAttackTimes.interp(val));
		env.set_envelope_time(Envelope::HOLD, toneHoldTimes.interp(val));
		env.set_envelope_time(Envelope::DECAY, toneDecayTimes.interp(val));
		env.set_envelope_time(Envelope::RELEASE, toneReleaseTimes.interp(val));
		env.set_sustain(toneBreakPoint.interp(val));
	}

	void setNoiseEnvelope(unsigned v) {
		float val = MathTools::constrain(baseNoiseEnvTime + voices[v].noiseEnvCV, 0.0f, 1.0f);
		auto &env = voices[v].envelopes[noiseEnvelope];
		env.set_envelope_time(Envelope::ATTACK, MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 50.0f));
		env.set_envelope_time(Envelope::DECAY, MathTools::map_value(val, 0.0f, 1.0f, 30.0f, 100.0f));
		env.set_envelope_time(Envelope::RELEASE, MathTools::map_value(val, 0.0f, 1.0f, 100.0f, 3000.0f));
		env.set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.25f));
	}

	void setPitchEnvelope(unsigned v) {
		float val = MathTools::constrain(voices[v].pitchEnvCV + basePitchEnvTime, 0.0f, 1.0f);
		auto &env = voices[v].envelopes[pitchEnvelope];
		env.set_envelope_time(Envelope::DECAY, pitchDecayTimes.interp(val));
		env.set_envelope_time(Envelope::RELEASE, pitchReleaseTimes.interp(val));
		env.set_sustain(pitchBreakPoint.interp(val));
	}

	void set_samplerate(float sr) override {
		for (auto &voice : voices) {
			for (int i = 0; i < 4; i++)
				voice.envelopes[i].set_samplerate(sr);
			voice.osc.set_samplerate(sr);
		}
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// If this voice's CV channel changed, store it and reconfigure that envelope
	void update_env_cv(unsigned v, float Voice::*cv_member, PolyJack const &jack, void (ThisCore::*setter)(unsigned)) {
		float cv = jack.or_last(v) / CvRangeVolts;
		if (cv != voices[v].*cv_member) {
			voices[v].*cv_member = cv;
			(this->*setter)(v);
		}
	}

	float noise(unsigned v) {
		noiseState[v] = noiseState[v] * 1103515245u + 12345u;
		return float(int32_t(noiseState[v])) * (1.f / 2147483648.f);
	}

	static constexpr float outputVolts = 5.f;
};

} // namespace MetaModule
