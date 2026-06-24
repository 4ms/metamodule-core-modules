#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "helpers/button_channel.hh"
#include "info/Octopush_info.hh"

#include <algorithm>
#include <array>
#include <utility>

namespace MetaModule
{

// Eight independent push-button voltage sources (see SoloPush) plus a mixer:
// the eight channel voltages are summed with a manual offset to produce Sum,
// Inverse Sum, and split Positive / Negative outputs.
class OctopushCore : public SmartCoreProcessor<OctopushInfo> {
	using Info = OctopushInfo;
	using ThisCore = OctopushCore;
	using enum Info::Elem;

	static constexpr unsigned NumChans = 8;

public:
	OctopushCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		// Per-channel processing; accumulate the channel voltages for the mixer.
		float sumVoltage = getState<MixerOffsetKnob>() * 10.f - 5.f;
		const float offsetVoltage = sumVoltage;

		[&]<unsigned... Ch>(std::integer_sequence<unsigned, Ch...>) {
			((sumVoltage += processChannel<Ch>()), ...);
		}(std::make_integer_sequence<unsigned, NumChans>{});

		sumVoltage = std::clamp(sumVoltage, -10.f, 10.f);
		setOutput<SumOut>(sumVoltage);
		setLED<SumLight>(BipolarColor_t{std::clamp(sumVoltage / 10.f, -1.f, 1.f)});

		const float invVoltage = -sumVoltage;
		setOutput<InvOut>(invVoltage);
		setLED<InvLight>(BipolarColor_t{std::clamp(invVoltage / 10.f, -1.f, 1.f)});

		const float posVoltage = sumVoltage > 0.f ? sumVoltage : 0.f;
		const float negVoltage = sumVoltage < 0.f ? sumVoltage : 0.f;
		setOutput<PosOut>(posVoltage);
		setOutput<NegOut>(negVoltage);
		setLED<PosLight>(std::min(posVoltage / 10.f, 1.f));
		setLED<NegLight>(std::min(-negVoltage / 10.f, 1.f));

		// Mixer offset LED: positive -> green, negative -> red.
		setLED<OffsetLight>(BipolarColor_t{std::clamp(offsetVoltage / 10.f, -1.f, 1.f)});
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
	// Process one channel; returns its (mixer-bound) voltage output.
	template<unsigned Ch>
	float processChannel() {
		constexpr auto el = [](Info::Elem first) constexpr { return static_cast<Info::Elem>(static_cast<int>(first) + Ch); };
		constexpr Info::Elem polarityEl = el(Ch1Polarity);
		constexpr Info::Elem rangeEl = el(Ch1Range);
		constexpr Info::Elem ampEl = el(Ch1Amplitude);
		constexpr Info::Elem behaviorEl = el(Ch1Behavior);
		constexpr Info::Elem pushEl = el(Ch1Push);
		constexpr Info::Elem voltOutEl = el(Ch1VoltageOut);
		constexpr Info::Elem logicOutEl = el(Ch1LogicOut);
		constexpr Info::Elem rgbEl = el(Ch1Rgb);

		const bool buttonPressed = getState<pushEl>() == MomentaryButton::State_t::PRESSED;
		const int mode = (int)getState<behaviorEl>();
		const bool polarity = getState<polarityEl>() != 0;
		const float amplitude = getState<ampEl>();
		const int range = (int)getState<rangeEl>();
		const float scale = ButtonUtils::RangeScales[range];

		auto out = channels[Ch].process(buttonPressed, mode, polarity, amplitude, scale, timeStepInS);

		setOutput<logicOutEl>(out.logicOut);
		setOutput<voltOutEl>(out.voltageOut);
		setLED<pushEl>(out.pushLedBrightness);
		// ledRed/ledGreen are mutually exclusive: positive -> green, negative -> red.
		setLED<rgbEl>(BipolarColor_t{out.ledGreen - out.ledRed});

		return out.voltageOut;
	}

	std::array<ButtonUtils::ButtonChannel, NumChans> channels{};
	float timeStepInS = 1.f / 48000.f;
};

} // namespace MetaModule
