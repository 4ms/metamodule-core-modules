#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/DEV_info.hh"

#include "CoreModules/4ms/core/envvca/FollowInput.h"
#include "CoreModules/4ms/core/envvca/SSI2162.h"
#include "CoreModules/4ms/core/envvca/Tables.h"
#include "CoreModules/4ms/core/envvca/TriangleOscillator.h"
#include "CoreModules/4ms/core/helpers/EdgeDetector.h"
#include "CoreModules/4ms/core/helpers/FlipFlop.h"
#include "CoreModules/4ms/core/helpers/circuit_elements.h"
#include "CoreModules/4ms/core/helpers/quantization.h"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

static float DEVProcessCVOffset(float slider, auto range) {
	// Slider plus resistor in parallel to tweak curve
	const float SliderImpedance = 100e3f;
	auto offset = 5.0f * VoltageDivider(slider * SliderImpedance + 17.4e3f,
										0 + ParallelCircuit(100e3f, (1.0f - slider) * SliderImpedance));

	// Select one of three bias voltages
	auto BiasFromRange = [](auto range) -> float {
		if (range == Toggle3pos::State_t::UP) {
			return -12.0f * VoltageDivider(1e3f, 10e3f);
		} else if (range == Toggle3pos::State_t::DOWN) {
			return 12.0f * VoltageDivider(1e3f, 8.2e3f);
		} else {
			// middle position, and fail-safe default
			return 0.0f;
		}
	};

	auto bias = BiasFromRange(range);

	return InvertingAmpWithBias(offset, 100e3f, 100e3f, bias);
}

// Polyphonic Dual EnvVCA. Each side (A/B) has two independently-polyphonic
// signal paths, like the ENVVCA: its Trig In sets the envelope channel count
// (Env Out and EOR/EOF Out), and its Audio In sets the audio channel count.
// Each audio channel is controlled by the envelope with the same channel
// number, with the highest envelope channel controlling all upper audio
// channels. CV inputs (Time CV, Cycle Gate, Follow, VCA CV) map onto channels
// the same way. Or Out carries the per-channel max of both sides' envelopes.
class DEVCore : public SmartCoreProcessorPoly<DEVInfo> {
	using Info = DEVInfo;
	using ThisCore = DEVCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	template<Info::Elem EL>
	void setOutput(float val, unsigned chan = 0) {
		SmartCoreProcessorPoly<Info>::setOutput<EL>(val, chan);
	}

	template<Info::Elem EL>
	std::optional<float> getInput(unsigned chan = 0) {
		return SmartCoreProcessorPoly<Info>::getInput<EL>(chan);
	}

	template<Info::Elem EL>
	unsigned numChannels() {
		return SmartCoreProcessorPoly<Info>::numChannels<EL>();
	}

	template<Info::Elem EL>
	bool isPatched() {
		return SmartCoreProcessorPoly<Info>::isPatched<EL>();
	}

	template<Info::Elem EL>
	void setChannels(unsigned chans) {
		SmartCoreProcessorPoly<Info>::setChannels<EL>(chans);
	}

	// Read an input channel, reusing the input's highest channel when it has
	// fewer channels than requested. Returns 0 if unpatched.
	template<Info::Elem EL>
	float getInputOrLast(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 0.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(0.f);
	}

	template<Info::Elem EL, typename VAL>
	void setLED(const VAL &value) {
		SmartCoreProcessorPoly<Info>::setLED<EL>(value);
	}

	template<Info::Elem EL>
	auto getState() {
		return SmartCoreProcessorPoly<Info>::getState<EL>();
	}

private:
	template<class Mapping>
	class Channel {
		static_assert(MaxChans == 4, "FlipFlop initializer below assumes 4 channels");

		std::array<SSI2162, MaxChans> vca{};
		std::array<TriangleOscillator, MaxChans> osc{};

		std::array<FlipFlop, MaxChans> triggerDetector{{{1.f, 2.f}, {1.f, 2.f}, {1.f, 2.f}, {1.f, 2.f}}};
		std::array<EdgeDetector, MaxChans> triggerEdgeDetector{};

		std::array<FollowInput, MaxChans> followInput{};

		std::array<float, MaxChans> envOut{};
		unsigned envChans = 1;

		float cycleLED = -1.f;

		float timeStepInS = 1.f / 48000.f;

		DEVCore *parent;

	public:
		Channel(DEVCore *parent_)
			: parent(parent_) {
		}

		unsigned numEnvChans() const {
			return envChans;
		}

		// Envelope output (after level/offset), highest channel feeding upward
		float getEnvOut(unsigned chan) const {
			return envOut[std::min(chan, envChans - 1)];
		}

		TriangleOscillator::SlopeState_t getOscillatorSlopeState(unsigned chan) const {
			return osc[std::min(chan, envChans - 1)].getSlopeState();
		}

		void update(unsigned audioChans, auto getAudioIn) {
			envChans = std::max(parent->numChannels<Mapping::TrigIn>(), 1u);
			parent->setChannels<Mapping::EnvOut>(envChans);

			runEnvelopes();
			runAudioPath(audioChans, getAudioIn);
		}

		void runEnvelopes() {
			// Channel-independent terms:
			const auto riseOffset = DEVProcessCVOffset(parent->getState<Mapping::RiseSlider>(),
													   parent->getState<Mapping::SlowMedFastRiseSwitch>());
			const auto fallOffset = DEVProcessCVOffset(parent->getState<Mapping::FallSlider>(),
													   parent->getState<Mapping::SlowMedFastFallSwitch>());
			const auto riseCvKnob = parent->getState<Mapping::RiseKnob>();
			const auto fallCvKnob = parent->getState<Mapping::FallKnob>();
			const auto levelScale = parent->getState<Mapping::LevelKnob>() * 2.0f - 1.0f;
			const auto offsetVolts = parent->getState<Mapping::OffsetKnob>() * 20.0f - 10.0f;
			const bool buttonCycling = parent->getState<Mapping::CycleButton>() == LatchingButton::State_t::DOWN;
			const bool followPatched = parent->isPatched<Mapping::FollowIn>();

			for (unsigned ch = 0; ch < envChans; ch++) {
				// Scale down CV input and apply attenuverter knobs
				const auto scaledTimeCV = parent->getInputOrLast<Mapping::TimeCvIn>(ch) * -100e3f / 137e3f;
				const auto rScale = InvertingAmpWithBias(scaledTimeCV, 100e3f, 100e3f, riseCvKnob * scaledTimeCV);
				const auto fScale = InvertingAmpWithBias(scaledTimeCV, 100e3f, 100e3f, fallCvKnob * scaledTimeCV);

				// Sum with static value from fader + range switch
				auto riseCV = -rScale - riseOffset;
				auto fallCV = -fScale - fallOffset;

				// Apply rise time limit and scale down
				constexpr float DiodeDropInV = 1.0f;
				const float ClippingVoltage = 5.0f * VoltageDivider(100e3f, 2e3f) + DiodeDropInV;
				riseCV = riseCV * VoltageDivider(2.2e3f + 33e3f, 16.9e3f);
				riseCV = std::min(riseCV, ClippingVoltage);
				riseCV = riseCV * VoltageDivider(2.2e3f, 33e3f);

				// Scale down falling CV without additional limiting
				fallCV = fallCV * VoltageDivider(2.2e3f, 10e3f + 40.2e3f);

				osc[ch].setRiseTimeInS(VoltageToTime(riseCV));
				osc[ch].setFallTimeInS(VoltageToTime(fallCV));

				const bool isCycling =
					buttonCycling ^ CVToBool(parent->getInputOrLast<Mapping::CycleTrig>(ch));
				osc[ch].setCycling(isCycling);

				if (followPatched) {
					osc[ch].setTargetVoltage(followInput[ch].process(parent->getInputOrLast<Mapping::FollowIn>(ch)));
				} else {
					osc[ch].setTargetVoltage(0.0f);
				}

				if (triggerEdgeDetector[ch](triggerDetector[ch](parent->getInput<Mapping::TrigIn>(ch).value_or(0.f)))) {
					osc[ch].doRetrigger();
				}

				osc[ch].proceed(timeStepInS);

				const auto envV = osc[ch].getOutput();
				const auto slopeState = osc[ch].getSlopeState();

				auto e = envV / VoltageDivider(100e3f, 100e3f);
				e = e * levelScale + offsetVolts;
				envOut[ch] = e;
				parent->setOutput<Mapping::EnvOut>(e, ch);

				// Panel LEDs show the first channel
				if (ch == 0) {
					parent->setLED<Mapping::RiseLedLight>(BipolarColor_t{-rScale / 10.f});
					parent->setLED<Mapping::FallLedLight>(BipolarColor_t{-fScale / 10.f});
					parent->setLED<Mapping::RiseSlider>(
						slopeState == TriangleOscillator::SlopeState_t::RISING ? envV / 8.f : 0);
					parent->setLED<Mapping::FallSlider>(
						slopeState == TriangleOscillator::SlopeState_t::FALLING ? envV / 8.f : 0);
					parent->setLED<Mapping::EnvLedLight>(BipolarColor_t{e / 8.f});
					if (cycleLED != isCycling) {
						cycleLED = isCycling;
						parent->setLED<Mapping::CycleButton>(cycleLED);
					}
				}
			}
		}

		void runAudioPath(unsigned audioChans, auto getAudioIn) {
			parent->setChannels<Mapping::AudioOut>(audioChans);

			const bool vcaCvPatched = parent->isPatched<Mapping::VcaCvIn>();

			for (unsigned ch = 0; ch < audioChans; ch++) {
				auto triangleWave = vcaCvPatched ? parent->getInputOrLast<Mapping::VcaCvIn>(ch) :
												   osc[std::min(ch, envChans - 1)].getOutput();

				triangleWave = InvertingAmpWithBias(triangleWave, 100e3f, 100e3f, 1.94f);

				constexpr float VCAInputImpendance = 5e3f;
				triangleWave = triangleWave * VoltageDivider(VCAInputImpendance, 2.7e3f);

				// This value influences the maximum gain a lot
				// Tweaked manually to achieve approximately max 0dB
				constexpr float SchottkyForwardVoltage = 0.22f;
				constexpr float MaxGainInV = 5.0f + SchottkyForwardVoltage;
				constexpr float MinGainInV = VoltageDivider(47e3f, 1e6f) * 5.0f - SchottkyForwardVoltage;

				triangleWave = std::clamp(triangleWave, MinGainInV, MaxGainInV);

				vca[ch].setScaling(triangleWave);

				parent->setOutput<Mapping::AudioOut>(vca[ch].process(getAudioIn(ch)), ch);
			}
		}

		void set_samplerate(float sr) {
			timeStepInS = 1.0f / sr;
		}
	};

private:
	struct MappingA {
		const static Info::Elem AudioIn = AudioAIn;
		const static Info::Elem AudioOut = AudioAOut;
		const static Info::Elem RiseSlider = RiseASlider;
		const static Info::Elem FallSlider = FallASlider;
		const static Info::Elem EnvOut = EnvAOut;
		const static Info::Elem EnvLedLight = EnvALight;
		const static Info::Elem LevelKnob = LevelAKnob;
		const static Info::Elem OffsetKnob = OffsetAKnob;
		const static Info::Elem CycleButton = CycleAButton;
		const static Info::Elem CycleTrig = CycleGateIn;
		const static Info::Elem FollowIn = FollowAIn;
		const static Info::Elem TrigIn = TrigAIn;
		const static Info::Elem RiseKnob = RiseAKnob;
		const static Info::Elem FallKnob = FallAKnob;
		const static Info::Elem TimeCvIn = TimeCvAIn;
		const static Info::Elem SlowMedFastRiseSwitch = RiseASwitch;
		const static Info::Elem SlowMedFastFallSwitch = FallASwitch;
		const static Info::Elem RiseLedLight = RiseALight;
		const static Info::Elem FallLedLight = FallALight;
		const static Info::Elem VcaCvIn = VcaCvAIn;
	};

	struct MappingB {
		const static Info::Elem AudioIn = AudioBIn;
		const static Info::Elem AudioOut = AudioBOut;
		const static Info::Elem RiseSlider = RiseBSlider;
		const static Info::Elem FallSlider = FallBSlider;
		const static Info::Elem EnvOut = EnvBOut;
		const static Info::Elem EnvLedLight = EnvBLight;
		const static Info::Elem LevelKnob = LevelBKnob;
		const static Info::Elem OffsetKnob = OffsetBKnob;
		const static Info::Elem CycleButton = CycleBButton;
		const static Info::Elem CycleTrig = CycleGateIn;
		const static Info::Elem FollowIn = FollowBIn;
		const static Info::Elem TrigIn = TrigBIn;
		const static Info::Elem RiseKnob = RiseBKnob;
		const static Info::Elem FallKnob = FallBKnob;
		const static Info::Elem TimeCvIn = TimeCvBIn;
		const static Info::Elem SlowMedFastRiseSwitch = RiseBSwitch;
		const static Info::Elem SlowMedFastFallSwitch = FallBSwitch;
		const static Info::Elem RiseLedLight = RiseBLight;
		const static Info::Elem FallLedLight = FallBLight;
		const static Info::Elem VcaCvIn = VcaCvBIn;
	};

	Channel<MappingA> channelA;
	Channel<MappingB> channelB;

	friend Channel<MappingA>;
	friend Channel<MappingB>;

public:
	DEVCore()
		: channelA(this)
		, channelB(this) {
	}

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const auto audioInA = [this](unsigned ch) { return getInput<MappingA::AudioIn>(ch).value_or(0.f); };
		const unsigned audioChansA = std::max(numChannels<MappingA::AudioIn>(), 1u);
		channelA.update(audioChansA, audioInA);

		// Audio In B is normalled to Audio In A
		if (isPatched<MappingB::AudioIn>()) {
			channelB.update(std::max(numChannels<MappingB::AudioIn>(), 1u),
							[this](unsigned ch) { return getInput<MappingB::AudioIn>(ch).value_or(0.f); });
		} else {
			channelB.update(audioChansA, audioInA);
		}

		displayOscillatorState();

		// Or Out: per-channel max of both sides' envelope outputs
		const unsigned orChans = std::max(channelA.numEnvChans(), channelB.numEnvChans());
		setChannels<OrOut>(orChans);
		for (unsigned ch = 0; ch < orChans; ch++)
			setOutput<OrOut>(std::max(channelA.getEnvOut(ch), channelB.getEnvOut(ch)), ch);
	}

	void displayOscillatorState() {
		const auto chansA = channelA.numEnvChans();
		setChannels<EorAOut>(chansA);
		for (unsigned ch = 0; ch < chansA; ch++) {
			const bool eor = channelA.getOscillatorSlopeState(ch) == TriangleOscillator::SlopeState_t::FALLING;
			setOutput<EorAOut>(eor ? 8.f : 0.f, ch);
		}
		setLED<EorLight>(channelA.getOscillatorSlopeState(0) == TriangleOscillator::SlopeState_t::FALLING);

		const auto chansB = channelB.numEnvChans();
		setChannels<EofBOut>(chansB);
		for (unsigned ch = 0; ch < chansB; ch++) {
			const bool eof = channelB.getOscillatorSlopeState(ch) != TriangleOscillator::SlopeState_t::FALLING;
			setOutput<EofBOut>(eof ? 8.f : 0.f, ch);
		}
		setLED<EofLight>(channelB.getOscillatorSlopeState(0) != TriangleOscillator::SlopeState_t::FALLING);
	}

	void set_samplerate(float sr) override {
		channelA.set_samplerate(sr);
		channelB.set_samplerate(sr);
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on
};

} // namespace MetaModule
