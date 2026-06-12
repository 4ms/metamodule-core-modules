#include "info/L4_info.hh"
#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "l4/Tables.h"
#include "l4/DCBlock.h"
#include "l4/PeakDetector.h"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: all outputs are as wide as the widest input; each input's
// highest channel feeds its upper voices. The stereo channels' Right inputs
// normal to their Left inputs. Per-voice DC blockers; the LED meters show the
// first channel.
class L4Core : public SmartCoreProcessorPoly<L4Info> {
	using Info = L4Info;
	using ThisCore = L4Core;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	L4Core() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = std::max({numChannels<Ch_1In>(),
									  numChannels<Ch_2In>(),
									  numChannels<Ch_3LeftIn>(),
									  numChannels<Ch_3RightIn>(),
									  numChannels<Ch_4LeftIn>(),
									  numChannels<Ch_4RightIn>(),
									  1u});
		setChannels<MainLeftOut>(nv);
		setChannels<MainRightOut>(nv);
		setChannels<HeadphoneOut>(nv);

		const auto ch1PanL = PanningTable.lookup(1.f - getState<Ch_1PanKnob>());
		const auto ch1PanR = PanningTable.lookup(getState<Ch_1PanKnob>());
		const auto ch1Level = LevelTable.lookup(getState<Ch_1LevelKnob>());
		const auto ch2PanL = PanningTable.lookup(1.f - getState<Ch_2PanKnob>());
		const auto ch2PanR = PanningTable.lookup(getState<Ch_2PanKnob>());
		const auto ch2Level = LevelTable.lookup(getState<Ch_2LevelKnob>());
		const auto ch3Level = LevelTable.lookup(getState<Ch_3LevelKnob>());
		const auto ch4Level = LevelTable.lookup(getState<Ch_4LevelKnob>());
		const auto headphoneLevel = LevelTable.lookup(getState<HeadphoneLevelKnob>());
		const auto mainLevel = LevelTable.lookup(getState<MainLevelKnob>());
		const bool lineMode = getState<OutputLevelSwitch>() == Toggle2posHoriz::State_t::RIGHT;
		const bool ch3RightPatched = isPatched<Ch_3RightIn>();
		const bool ch4RightPatched = isPatched<Ch_4RightIn>();

		for (unsigned v = 0; v < nv; v++) {
			auto &dc = blockers[v];

			float outputLeft = 0.f;
			float outputRight = 0.f;

			float ch1L, ch1R, ch2L, ch2R, ch3L, ch3R, ch4L, ch4R;

			{
				auto filteredInput = dc.ch1(getInputOrLast<Ch_1In>(v));
				ch1L = filteredInput * ch1PanL * ch1Level;
				ch1R = filteredInput * ch1PanR * ch1Level;
				outputLeft += ch1L;
				outputRight += ch1R;
			}

			{
				auto filteredInput = dc.ch2(getInputOrLast<Ch_2In>(v));
				ch2L = filteredInput * ch2PanL * ch2Level;
				ch2R = filteredInput * ch2PanR * ch2Level;
				outputLeft += ch2L;
				outputRight += ch2R;
			}

			{
				ch3L = dc.ch3L(getInputOrLast<Ch_3LeftIn>(v)) * ch3Level;
				ch3R = ch3RightPatched ? dc.ch3R(getInputOrLast<Ch_3RightIn>(v)) * ch3Level : ch3L;
				outputLeft += ch3L;
				outputRight += ch3R;
			}

			{
				ch4L = dc.ch4L(getInputOrLast<Ch_4LeftIn>(v)) * ch4Level;
				ch4R = ch4RightPatched ? dc.ch4R(getInputOrLast<Ch_4RightIn>(v)) * ch4Level : ch4L;
				outputLeft += ch4L;
				outputRight += ch4R;
			}

			//+6dB output boost
			outputLeft *= 2.f;
			outputRight *= 2.f;

			auto headphoneOut = (outputLeft + outputRight) * headphoneLevel;

			//-16.2dB attenuation in line mode
			if (lineMode) {
				outputLeft *= 0.155f;
				outputRight *= 0.155f;
			}

			outputLeft *= mainLevel;
			outputRight *= mainLevel;

			setOutput<MainLeftOut>(std::clamp(outputLeft, -11.f, 11.f), v);
			setOutput<MainRightOut>(std::clamp(outputRight, -11.f, 11.f), v);
			setOutput<HeadphoneOut>(std::clamp(headphoneOut, -11.f, 11.f), v);

			// LED meters show the first channel
			if (v == 0) {
				setLED<Ch_1LevelLedLight>(std::array<float, 3>{
					0.f, channel1EnvelopeRight(gcem::abs(ch1R)) / LEDScaling, channel1EnvelopeLeft(gcem::abs(ch1L)) / LEDScaling});
				setLED<Ch_2LevelLedLight>(std::array<float, 3>{
					0.f, channel2EnvelopeRight(gcem::abs(ch2R)) / LEDScaling, channel2EnvelopeLeft(gcem::abs(ch2L)) / LEDScaling});
				setLED<Ch_3LevelLedLight>(std::array<float, 3>{
					0.f, channel3EnvelopeRight(gcem::abs(ch3R)) / LEDScaling, channel3EnvelopeLeft(gcem::abs(ch3L)) / LEDScaling});
				setLED<Ch_4LevelLedLight>(std::array<float, 3>{
					0.f, channel4EnvelopeRight(gcem::abs(ch4R)) / LEDScaling, channel4EnvelopeLeft(gcem::abs(ch4L)) / LEDScaling});

				auto outputLeftEnvelope = mainEnvelopeLeft(gcem::abs(outputLeft));
				auto outputRightEnvelope = mainEnvelopeRight(gcem::abs(outputRight));
				auto clippingLeft = outputLeftEnvelope >= 10.f ? 1.f : 0.f;
				auto clippingRight = outputRightEnvelope >= 10.f ? 1.f : 0.f;

				setLED<MainOutLeftLedLight>(std::array<float, 3>{clippingLeft, 0.0f, outputLeftEnvelope / LEDScaling});
				setLED<MainOutRightLedLight>(std::array<float, 3>{clippingRight, outputRightEnvelope / LEDScaling, 0.f});
			}
		}
	}

	void set_samplerate(float sr) override {
		mainEnvelopeLeft.setSamplerate(sr);
		mainEnvelopeRight.setSamplerate(sr);
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// Read an input channel, reusing the input's highest channel when it has
	// fewer channels than requested. Returns 0 if unpatched.
	template<Info::Elem EL>
	float getInputOrLast(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 0.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(0.f);
	}

	static constexpr float LEDScaling = 5.f;
	static constexpr float DCBlockerFactor = 0.9995f;

	struct VoiceBlockers {
		DCBlock ch1{DCBlockerFactor};
		DCBlock ch2{DCBlockerFactor};
		DCBlock ch3L{DCBlockerFactor};
		DCBlock ch3R{DCBlockerFactor};
		DCBlock ch4L{DCBlockerFactor};
		DCBlock ch4R{DCBlockerFactor};
	};
	std::array<VoiceBlockers, MaxChans> blockers{};

	PeakDetector channel1EnvelopeLeft;
	PeakDetector channel1EnvelopeRight;

	PeakDetector channel2EnvelopeLeft;
	PeakDetector channel2EnvelopeRight;

	PeakDetector channel3EnvelopeLeft;
	PeakDetector channel3EnvelopeRight;

	PeakDetector channel4EnvelopeLeft;
	PeakDetector channel4EnvelopeRight;

	PeakDetector mainEnvelopeLeft;
	PeakDetector mainEnvelopeRight;
};

} // namespace MetaModule
