#include "CoreModules/4ms/info/ENVVCA_info.hh"
#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"

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

namespace MetaModule
{

// Polyphonic EnvVCA with two independently-polyphonic signal paths:
//  - Trigger In -> Env Out (+ EOR Out): one envelope per Trigger In channel
//  - Audio In -> Audio Out: one VCA channel per Audio In channel
// Each audio channel is controlled by the envelope with the same channel
// number; if there are fewer envelopes than audio channels, the highest
// envelope channel controls all the upper audio channels. The CV inputs
// (Time CV, Cycle, Follow) map onto envelope channels the same way.
class ENVVCACore : public SmartCoreProcessorPoly<ENVVCAInfo> {
	using Info = ENVVCAInfo;
	using ThisCore = ENVVCACore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	ENVVCACore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned envChans = std::max(numChannels<TriggerIn>(), 1u);
		const unsigned audioChans = std::max(numChannels<AudioIn>(), 1u);
		setChannels<EnvOut>(envChans);
		setChannels<EorOut>(envChans);
		setChannels<AudioOut>(audioChans);

		runEnvelopes(envChans);
		runAudioPath(envChans, audioChans);
	}

	void runEnvelopes(unsigned envChans) {
		// Channel-independent terms:
		const auto riseOffset = ProcessCVOffset(getState<RiseSlider>(), getState<RiseRangeSwitch>());
		const auto fallOffset = ProcessCVOffset(getState<FallSlider>(), getState<FallRangeSwitch>());
		const auto riseCvKnob = getState<RiseCvKnob>();
		const auto fallCvKnob = getState<FallCvKnob>();
		const auto envLevel = getState<EnvLevelSlider>();
		const bool buttonCycling = getState<CycleButton>() == LatchingButton::State_t::DOWN;
		const bool followPatched = isPatched<FollowIn>();

		for (unsigned ch = 0; ch < envChans; ch++) {
			// Scale down CV input and apply attenuverter knobs
			const auto scaledTimeCV = getInputOrLast<TimeCvIn>(ch) * -100e3f / 137e3f;
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

			// Cycle, Follow, and Trigger (mapped per envelope channel):
			const bool isCycling = buttonCycling ^ CVToBool(getInputOrLast<CycleIn>(ch));
			osc[ch].setCycling(isCycling);

			if (followPatched) {
				osc[ch].setTargetVoltage(followInput[ch].process(getInputOrLast<FollowIn>(ch)));
			}

			if (triggerEdgeDetector[ch](triggerDetector[ch](getInput<TriggerIn>(ch).value_or(0.f)))) {
				osc[ch].doRetrigger();
			}

			osc[ch].proceed(timeStepInS);

			const auto envV = osc[ch].getOutput();
			const auto slopeState = osc[ch].getSlopeState();
			const bool falling = slopeState == TriangleOscillator::SlopeState_t::FALLING;

			setOutput<EorOut>(falling ? 8.f : 0.f, ch);

			const auto envOutV = envV / VoltageDivider(100e3f, 100e3f) * envLevel;
			setOutput<EnvOut>(envOutV, ch);

			// Panel LEDs show the first channel
			if (ch == 0) {
				setLED<RiseLight>(BipolarColor_t{-rScale / 7.5f});
				setLED<FallLight>(BipolarColor_t{-fScale / 7.5f});
				setLED<RiseSlider>(slopeState == TriangleOscillator::SlopeState_t::RISING ? envV / 8.f : 0);
				setLED<FallSlider>(falling ? envV / 8.f : 0);
				setLED<EnvLevelSlider>(envOutV / 8.f);
				setLED<EorLight>(falling);
				if (cycleLED != isCycling) {
					cycleLED = isCycling;
					setLED<CycleButton>(cycleLED);
				}
			}
		}
	}

	void runAudioPath(unsigned envChans, unsigned audioChans) {
		// One VCA gain per envelope channel (upper audio channels reuse the highest)
		for (unsigned ch = 0; ch < std::min(envChans, audioChans); ch++) {
			auto triangleWave = InvertingAmpWithBias(osc[ch].getOutput(), 100e3f, 100e3f, 1.94f);

			constexpr float VCAInputImpendance = 5e3f;
			triangleWave = triangleWave * VoltageDivider(VCAInputImpendance, 2.7e3f);

			// This value influences the maximum gain a lot
			// Tweaked manually to achieve approximately max 0dB
			constexpr float SchottkyForwardVoltage = 0.22f;
			constexpr float MaxGainInV = 5.0f + SchottkyForwardVoltage;
			constexpr float MinGainInV = VoltageDivider(47e3f, 1e6f) * 5.0f - SchottkyForwardVoltage;

			triangleWave = std::clamp(triangleWave, MinGainInV, MaxGainInV);

			vca[ch].setScaling(triangleWave);
		}

		// Ignoring input impedance and inverting 400kHz lowpass
		for (unsigned ch = 0; ch < audioChans; ch++) {
			auto &channelVca = vca[std::min(ch, envChans - 1)];
			auto input = getInput<AudioIn>(ch).value_or(0.f);
			setOutput<AudioOut>(channelVca.process(input), ch);
		}
		// Ignoring output impedance and inverting 400kHz lowpass
	}

	void set_samplerate(float sr) override {
		timeStepInS = 1.0f / sr;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// Read an input channel, reusing the input's highest channel when it has
	// fewer channels than the envelope path. Returns 0 if unpatched.
	template<Info::Elem EL>
	float getInputOrLast(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 0.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(0.f);
	}

	static float ProcessCVOffset(float slider, auto range) {
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

private:
	float cycleLED = -1.f;

	static_assert(MaxChans == 4, "FlipFlop initializer below assumes 4 channels");
	std::array<FlipFlop, MaxChans> triggerDetector{{{1.f, 2.f}, {1.f, 2.f}, {1.f, 2.f}, {1.f, 2.f}}};
	std::array<EdgeDetector, MaxChans> triggerEdgeDetector{};
	std::array<FollowInput, MaxChans> followInput{};

	std::array<TriangleOscillator, MaxChans> osc{};
	std::array<SSI2162, MaxChans> vca{};

	float timeStepInS = 1.f / 48000.f;
};

} // namespace MetaModule
