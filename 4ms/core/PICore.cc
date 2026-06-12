#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "helpers/envelope_follower.hh"
#include "info/PI_info.hh"

#include "l4/DCBlock.h"
#include "l4/PeakDetector.h"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// follower, gate, and DC blocker. The panel LEDs show the first channel.
class PICore : public SmartCoreProcessorPoly<PIInfo> {
	using Info = PIInfo;
	using ThisCore = PICore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

private:
	enum GainRange_t { LOW = 0, MEDIUM = 1, HIGH = 2 };
	enum GateState_t { IDLE, TRIGGERED };
	enum Mode_t { FOLLOW, GEN };

public:
	PICore() {
		for (auto &voice : voices)
			voice.envelope.setAttack(0.005f);
	}

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		ticks++;

		const unsigned nv = std::max(numChannels<AudioIn>(), 1u);
		setChannels<GateOut>(nv);
		setChannels<EnvPOut>(nv);
		setChannels<EnvNOut>(nv);
		setChannels<Env_Out>(nv);
		setChannels<InvertedOut>(nv);
		setChannels<AudioOut>(nv);

		const bool audioPatched = isPatched<AudioIn>();
		const auto maximumGain = readMaximumGain();
		const auto sensitivity = getState<SensitivityKnob>();
		const auto envLevel = getState<EnvLevelKnob>();
		const auto invertedLevel = getState<InvertedLevelKnob>();
		const auto decayTime =
			getState<EnvDecayKnob>() * (maximumDecayTimeInS - minimumDecayTimeInS) + minimumDecayTimeInS;
		readEnvelopeMode();

		for (unsigned v = 0; v < nv; v++) {
			auto &voice = voices[v];

			auto scaledInput = 0.f;

			if (audioPatched) {
				auto filteredInput = voice.dcBlocker(getInput<AudioIn>(v).value_or(0.f));
				scaledInput = std::clamp(filteredInput * (sensitivity * maximumGain), -12.f, 12.f);

				checkGateTrigger(voice, scaledInput);
			}

			updateGate(voice, ticks);

			voice.envelope.setDecay(decayTime);

			auto envelopePOut = generateEnvelope(voice,
												 mode == FOLLOW				 ? scaledInput :
												 voice.gateState == TRIGGERED ? gateOutHighVoltage :
																			   gateOutLowVoltage);

			auto envelopeNOut = envelopeHighVoltage - envelopePOut;

			setOutput<GateOut>(voice.gateState == TRIGGERED ? gateOutHighVoltage : gateOutLowVoltage, v);
			setOutput<EnvPOut>(envelopePOut, v);
			setOutput<EnvNOut>(envelopeNOut, v);
			setOutput<Env_Out>(envelopePOut * envLevel, v);
			setOutput<InvertedOut>(envelopeNOut * invertedLevel, v);
			setOutput<AudioOut>(scaledInput, v);

			// Panel LEDs show the first channel
			if (v == 0) {
				setLED<GateLight>(voice.gateState == TRIGGERED ? 1.f : 0.f);
				setLED<EnvPLight>(envelopePOut / envelopeHighVoltage);
				setLED<EnvNLight>(envelopeNOut / envelopeHighVoltage);
				auto sensLight = voice.senseEnvelope(scaledInput);
				setLED<Sens_Light>(std::array<float, 3>{(sensLight - 6.f) / 3.f, 0.f, sensLight / 3.5f});
			}
		}
	}

	float readMaximumGain() {
		auto gainMode = getState<GainSwitch>();

		if (gainMode == Toggle3posHoriz::State_t::LEFT) {
			return maximumGains[LOW];
		} else if (gainMode == Toggle3posHoriz::State_t::CENTER) {
			return maximumGains[MEDIUM];
		} else {
			return maximumGains[HIGH];
		}
	}

	void readEnvelopeMode() {
		auto envMode = getState<EnvModeSwitch>();

		if (envMode == Toggle2posHoriz::State_t::LEFT) {
			mode = FOLLOW;
		} else {
			mode = GEN;
		}
	}

	void set_samplerate(float sr) override {
		for (auto &voice : voices) {
			voice.envelope.setSamplerate(sr);
			voice.senseEnvelope.setSamplerate(sr);
		}
		sampleRate = sr;
		minimumGateLengthInTicks = minimumGateLengthInS * sampleRate;
		maximumGateLengthInTicks = maximumGateLengthInS * sampleRate;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	static constexpr float minimumGateLengthInS = 0.005f;
	static constexpr float maximumGateLengthInS = 0.5f;

	static constexpr float gateOutLowVoltage = 0.f;
	static constexpr float gateOutHighVoltage = 8.f;

	static constexpr float gateThresholdInV = 3.5f;

	static constexpr float envelopeHighVoltage = 9.f;

	static constexpr float minimumDecayTimeInS = 0.022f;
	static constexpr float maximumDecayTimeInS = 2.2f;

	static constexpr std::array<float, 3> maximumGains{2.f, 20.f, 500.f};

	static constexpr float DCBlockerFactor = 0.9995f;

private:
	struct Voice {
		DCBlock dcBlocker{DCBlockerFactor};
		EnvelopeFollower envelope;
		PeakDetector senseEnvelope;
		GateState_t gateState = IDLE;
		uint32_t lastGateTriggerInTicks = 0;
	};

	void checkGateTrigger(Voice &voice, float input) {
		if (voice.gateState == IDLE) {
			if (input >= gateThresholdInV) {
				voice.gateState = TRIGGERED;
				voice.lastGateTriggerInTicks = ticks;
			}
		}
	}

	void updateGate(Voice &voice, uint32_t now) {
		if (voice.gateState == TRIGGERED) {
			auto gateLengthInTicks = getState<SustainKnob>() * (maximumGateLengthInTicks - minimumGateLengthInTicks) +
									 minimumGateLengthInTicks;

			if (now > voice.lastGateTriggerInTicks + gateLengthInTicks) {
				voice.gateState = IDLE;
			}
		}
	}

	static float generateEnvelope(Voice &voice, float input) {
		static constexpr float envelopeInputGain = 3.2f;
		static constexpr float envelopeInputOffset = -0.29f;

		input *= envelopeInputGain;
		input += envelopeInputOffset;

		return voice.envelope(std::clamp(input, 0.f, envelopeHighVoltage));
	}

	uint32_t ticks = 0;
	float sampleRate = 48000.f;
	Mode_t mode = FOLLOW;

	uint32_t minimumGateLengthInTicks = minimumGateLengthInS * 48000;
	uint32_t maximumGateLengthInTicks = maximumGateLengthInS * 48000;

	std::array<Voice, MaxChans> voices{};
};

} // namespace MetaModule
