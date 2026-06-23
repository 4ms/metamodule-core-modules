#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/FadeIn_info.hh"

#include <cmath>

namespace MetaModule
{

// Stereo "fade in" gain: pressing the Fade In button ramps the gain from 0 to
// 1 over the Fade Time; the Shape knob morphs the ramp from linear (0) toward
// logarithmic (+1). The Reset button forces the gain back to 0. Audio Right is
// normalled to Audio Left when its jack is unpatched.
class FadeInCore : public SmartCoreProcessor<FadeInInfo> {
	using Info = FadeInInfo;
	using ThisCore = FadeInCore;
	using enum Info::Elem;

public:
	FadeInCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const bool buttonPressed = getState<FadeButton>() == MomentaryButton::State_t::PRESSED;
		const bool risingEdge = buttonPressed && !prevButtonState;
		prevButtonState = buttonPressed;

		const bool resetPressed = getState<ResetButton>() == MomentaryButton::State_t::PRESSED;
		const bool resetRisingEdge = resetPressed && !prevResetState;
		prevResetState = resetPressed;

		if (resetRisingEdge) {
			isActive = false;
			fadeProgress = 0.f;
			currentGain = 0.f;
			resetLedTimer = 0.1f;
		}

		if (risingEdge) {
			isActive = true;
			fadeProgress = 0.f;
			currentGain = 0.f;
		}

		if (isActive) {
			// Knobs report a raw 0..1 value; map to the panel's display ranges.
			const float fadeTime = 0.5f + getState<FadeTimeKnob>() * (10.f - 0.5f);
			const float shape = getState<ShapeKnob>() * 2.f - 1.f;

			fadeProgress += timeStepInS / fadeTime;
			if (fadeProgress >= 1.f) {
				fadeProgress = 1.f;
				isActive = false;
			}

			const float t = fadeProgress;
			const float linear = t;
			const float logarithmic = std::log(1.f + t * 9.f) / std::log(10.f);
			const float shaped = linear + (logarithmic - linear) * shape;
			currentGain = shaped;
		}

		const float inL = getInput<AudioInL>().value_or(0.f);
		const float inR = getInput<AudioInR>().value_or(0.f);
		const float outR = isPatched<AudioInR>() ? inR : inL;

		const float outLVoltage = inL * currentGain;
		const float outRVoltage = outR * currentGain;
		setOutput<AudioOutL>(outLVoltage);
		setOutput<AudioOutR>(outRVoltage);

		// Fade In button LED follows the gain; Reset LED flashes briefly on reset.
		setLED<FadeButton>(currentGain);

		if (resetLedTimer > 0.f)
			resetLedTimer -= timeStepInS;
		setLED<ResetButton>(resetLedTimer > 0.f ? 1.f : 0.f);

		setLED<InLLight>(std::abs(inL) / 5.f);
		setLED<InRLight>(std::abs(inR) / 5.f);
		setLED<OutLLight>(std::abs(outLVoltage) / 5.f);
		setLED<OutRLight>(std::abs(outRVoltage) / 5.f);
	}

	void set_samplerate(float sr) override {
		timeStepInS = 1.f / sr;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType("4msCompany", Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	bool prevButtonState = false;
	bool prevResetState = false;
	float resetLedTimer = 0.f;
	bool isActive = false;
	float fadeProgress = 0.f;
	float currentGain = 0.f;

	float timeStepInS = 1.f / 48000.f;
};

} // namespace MetaModule
