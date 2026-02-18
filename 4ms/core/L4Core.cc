#include "info/L4_info.hh"
#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "l4/Tables.h"
#include "l4/DCBlock.h"
#include "l4/PeakDetector.h"

#include "processors/tools/expDecay.h"


namespace MetaModule
{

class L4Core : public SmartCoreProcessor<L4Info> {
	using Info = L4Info;
	using ThisCore = L4Core;
	using enum Info::Elem;

public:
	L4Core() :
		channel1DCBlocker(DCBlockerFactor), channel2DCBlocker(DCBlockerFactor), 
		channel3LeftDCBlocker(DCBlockerFactor), channel3RightDCBlocker(DCBlockerFactor),
		channel4LeftDCBlocker(DCBlockerFactor), channel4RightDCBlocker(DCBlockerFactor) {
	};

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		float outputLeft = 0.f;
		float outputRight = 0.f;

		{
			auto input = getInput<Ch_1In>().value_or(0.f);
			auto filteredInput = channel1DCBlocker(input);
			auto channelLeft = filteredInput * PanningTable.lookup(1.f - getState<Ch_1PanKnob>()) * LevelTable.lookup(getState<Ch_1LevelKnob>());
			auto channelRight = filteredInput * PanningTable.lookup(getState<Ch_1PanKnob>()) * LevelTable.lookup(getState<Ch_1LevelKnob>());

			outputLeft += channelLeft;
			outputRight += channelRight;

			setLED<Ch_1LevelLedLight>(std::array<float,3>{0.f, channel1EnvelopeRight(gcem::abs(channelRight)) / LEDScaling , channel1EnvelopeLeft(gcem::abs(channelLeft)) / LEDScaling});
		}

		{
			auto input = getInput<Ch_2In>();
			auto filteredInput = channel2DCBlocker(*input);
			auto channelLeft = filteredInput * PanningTable.lookup(1.f - getState<Ch_2PanKnob>()) * LevelTable.lookup(getState<Ch_2LevelKnob>());
			auto channelRight = filteredInput * PanningTable.lookup(getState<Ch_2PanKnob>()) * LevelTable.lookup(getState<Ch_2LevelKnob>());

			outputLeft += channelLeft;
			outputRight += channelRight;

			setLED<Ch_2LevelLedLight>(std::array<float,3>{0.f, channel2EnvelopeRight(gcem::abs(channelRight)) / LEDScaling , channel2EnvelopeLeft(gcem::abs(channelLeft)) / LEDScaling});
		}

		{
			float channelLeft = 0.f;
			float channelRight = 0.f;

			auto inputL = getInput<Ch_3LeftIn>().value_or(0.f);
			auto filteredInput = channel3LeftDCBlocker(inputL);
			channelLeft = filteredInput * LevelTable.lookup(getState<Ch_3LevelKnob>());

			if(auto inputR = getInput<Ch_3RightIn>(); inputR) {
				auto filteredInput = channel3RightDCBlocker(*inputR);
				channelRight = filteredInput * LevelTable.lookup(getState<Ch_3LevelKnob>());

			} else
				channelRight =  channelLeft;

			outputLeft += channelLeft;
			outputRight += channelRight;

			setLED<Ch_3LevelLedLight>(std::array<float,3>{0.f, channel3EnvelopeRight(gcem::abs(channelRight)) / LEDScaling , channel3EnvelopeLeft(gcem::abs(channelLeft)) / LEDScaling});
		}

		{
			float channelLeft = 0.f;
			float channelRight = 0.f;

			auto inputL = getInput<Ch_4LeftIn>().value_or(0.f);
			auto filteredInput = channel4LeftDCBlocker(inputL);
			channelLeft = filteredInput * LevelTable.lookup(getState<Ch_4LevelKnob>());

			if(auto inputR = getInput<Ch_4RightIn>(); inputR) {
				auto filteredInput = channel4RightDCBlocker(*inputR);
				channelRight = filteredInput * LevelTable.lookup(getState<Ch_4LevelKnob>());

			} else 
				channelRight = channelLeft;

			outputLeft += channelLeft;
			outputRight += channelRight;

			setLED<Ch_4LevelLedLight>(std::array<float,3>{0.f, channel4EnvelopeRight(gcem::abs(channelRight)) / LEDScaling , channel4EnvelopeLeft(gcem::abs(channelLeft)) / LEDScaling});
		}

		//+6dB output boost
		outputLeft *= 2.f;
		outputRight *= 2.f;

		auto headphoneOut = (outputLeft + outputRight) * LevelTable.lookup(getState<HeadphoneLevelKnob>());

		//-16.2dB attenuation in line mode
		if(getState<OutputLevelSwitch>() == Toggle2posHoriz::State_t::RIGHT) {
			outputLeft *= 0.155f;
			outputRight *= 0.155f;
		}

		outputLeft *= LevelTable.lookup(getState<MainLevelKnob>());
		outputRight *= LevelTable.lookup(getState<MainLevelKnob>());

		auto outputLeftEnvelope = mainEnvelopeLeft(gcem::abs(outputLeft));
		auto outputRightEnvelope = mainEnvelopeRight(gcem::abs(outputRight));
		auto clippingLeft = outputLeftEnvelope >= 10.f ? 1.f : 0.f;
		auto clippingRight = outputRightEnvelope >= 10.f ? 1.f : 0.f;

		setLED<MainOutLeftLedLight>(std::array<float,3>{clippingLeft, 0.0f, outputLeftEnvelope / LEDScaling});
		setLED<MainOutRightLedLight>(std::array<float,3>{clippingRight, outputRightEnvelope / LEDScaling, 0.f});

		setOutput<MainLeftOut>(std::clamp(outputLeft, -11.f, 11.f));
		setOutput<MainRightOut>(std::clamp(outputRight, -11.f, 11.f));
		setOutput<HeadphoneOut>(std::clamp(headphoneOut, -11.f, 11.f));
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
	static constexpr float LEDScaling = 5.f;
	static constexpr float envelopeTimeConstant = 2000.f;
	static constexpr float DCBlockerFactor = 0.9995f;

	DCBlock channel1DCBlocker;
	DCBlock channel2DCBlocker;
	DCBlock channel3LeftDCBlocker;
	DCBlock channel3RightDCBlocker;
	DCBlock channel4LeftDCBlocker;
	DCBlock channel4RightDCBlocker;	

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
