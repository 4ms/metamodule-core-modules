#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "helpers/button_channel.hh"
#include "info/SoloPush_info.hh"

namespace MetaModule
{

// Single-channel push-button voltage source. The button produces a logic
// signal (Gate / Toggle / Trig depending on the Behavior switch) and a scaled
// voltage (Amplitude knob x Range switch, optionally bipolar).
class SoloPushCore : public SmartCoreProcessor<SoloPushInfo> {
	using Info = SoloPushInfo;
	using ThisCore = SoloPushCore;
	using enum Info::Elem;

public:
	SoloPushCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const bool buttonPressed = getState<PushButton>() == MomentaryButton::State_t::PRESSED;
		const int mode = getState<BehaviorSwitch>();
		const bool polarity = getState<PolaritySwitch>() != 0;
		const float amplitude = getState<AmplitudeKnob>();
		const int range = getState<RangeSwitch>();
		const float scale = ButtonUtils::RangeScales[range];

		auto out = channel.process(buttonPressed, mode, polarity, amplitude, scale, timeStepInS);

		setOutput<LogicOut>(out.logicOut);
		setOutput<VoltageOut>(out.voltageOut);
		setLED<PushButton>(out.pushLedBrightness);
		// ledRed/ledGreen are mutually exclusive: positive -> green, negative -> red.
		setLED<RgbLight>(BipolarColor_t{out.ledGreen - out.ledRed});
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
	ButtonUtils::ButtonChannel channel;
	float timeStepInS = 1.f / 48000.f;
};

} // namespace MetaModule
