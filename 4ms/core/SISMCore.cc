#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/SISM_info.hh"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: each channel's output follows its (possibly normalled) input's
// channel count; the slice/mix outputs are as wide as the widest channel.
// Channel 2 normals to channel 1, and channel 4 normals to channel 3.
class SISMCore : public SmartCoreProcessorPoly<SISMInfo> {
	using Info = SISMInfo;
	using ThisCore = SISMCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	SISMCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		// Effective per-channel source counts (with normalling):
		const unsigned chans1 = std::max(numChannels<Ch_1In>(), 1u);
		const unsigned chans2 = isPatched<Ch_2In>() ? numChannels<Ch_2In>() : chans1;
		const unsigned chans3 = std::max(numChannels<Ch_3In>(), 1u);
		const unsigned chans4 = isPatched<Ch_4In>() ? numChannels<Ch_4In>() : chans3;

		const unsigned nv = std::max({chans1, chans2, chans3, chans4});

		setChannels<Ch_1Out>(chans1);
		setChannels<Ch_2Out>(chans2);
		setChannels<Ch_3Out>(chans3);
		setChannels<Ch_4Out>(chans4);
		setChannels<PSliceOut>(nv);
		setChannels<NSliceOut>(nv);
		setChannels<MixOut>(nv);
		setChannels<Mix_Sw_Out>(nv);

		const std::array<float, 4> scale{getState<Ch_1ScaleKnob>(),
										 getState<Ch_2ScaleKnob>(),
										 getState<Ch_3ScaleKnob>(),
										 getState<Ch_4ScaleKnob>()};
		const std::array<float, 4> shift{getState<Ch_1ShiftKnob>(),
										 getState<Ch_2ShiftKnob>(),
										 getState<Ch_3ShiftKnob>(),
										 getState<Ch_4ShiftKnob>()};
		const std::array<bool, 4> outPatched{
			isPatched<Ch_1Out>(), isPatched<Ch_2Out>(), isPatched<Ch_3Out>(), isPatched<Ch_4Out>()};

		for (unsigned v = 0; v < nv; v++) {
			std::array<float, 4> inputValue;
			inputValue[0] = getInputOrLast<Ch_1In>(v);
			inputValue[1] = isPatched<Ch_2In>() ? getInputOrLast<Ch_2In>(v) : inputValue[0];
			inputValue[2] = getInputOrLast<Ch_3In>(v);
			inputValue[3] = isPatched<Ch_4In>() ? getInputOrLast<Ch_4In>(v) : inputValue[2];

			std::array<float, 4> outputValue;
			for (unsigned i = 0; i < 4; i++)
				outputValue[i] = process(inputValue[i], scale[i], shift[i]);

			setOutput<Ch_1Out>(std::clamp(outputValue[0], minimumOutputInV, maximumOutputInV), v);
			setOutput<Ch_2Out>(std::clamp(outputValue[1], minimumOutputInV, maximumOutputInV), v);
			setOutput<Ch_3Out>(std::clamp(outputValue[2], minimumOutputInV, maximumOutputInV), v);
			setOutput<Ch_4Out>(std::clamp(outputValue[3], minimumOutputInV, maximumOutputInV), v);

			auto slicePositive = 0.f;
			auto sliceNegative = 0.f;
			auto mixOut = 0.f;
			auto mixOutSW = 0.f;

			for (auto index = 0u; index < outputValue.size(); index++) {
				slicePositive += std::clamp(outputValue[index], 0.f, maximumOutputInV);
				sliceNegative += std::clamp(outputValue[index], minimumOutputInV, 0.f);
				mixOut += outputValue[index];
				mixOutSW += outPatched[index] ? 0.f : outputValue[index];
			}

			setOutput<PSliceOut>(std::clamp(slicePositive, 0.f, maximumOutputInV), v);
			setOutput<NSliceOut>(std::clamp(sliceNegative, minimumOutputInV, 0.f), v);
			setOutput<MixOut>(std::clamp(mixOut, minimumOutputInV, maximumOutputInV), v);
			setOutput<Mix_Sw_Out>(std::clamp(mixOutSW, minimumOutputInV, maximumOutputInV), v);

			// Panel LEDs show the first channel
			if (v == 0) {
				setLED<LedN1Light>(outputValue[0] / negativeLEDScaling);
				setLED<LedP1Light>(outputValue[0] / positiveLEDScaling);
				setLED<LedN2Light>(outputValue[1] / negativeLEDScaling);
				setLED<LedP2Light>(outputValue[1] / positiveLEDScaling);
				setLED<LedN3Light>(outputValue[2] / negativeLEDScaling);
				setLED<LedP3Light>(outputValue[2] / positiveLEDScaling);
				setLED<LedN4Light>(outputValue[3] / negativeLEDScaling);
				setLED<LedP4Light>(outputValue[3] / positiveLEDScaling);
				setLED<LedPSliceLight>(slicePositive / positiveLEDScaling);
				setLED<LedNSliceLight>(sliceNegative / negativeLEDScaling);
				setLED<LedNMixLight>(mixOut / negativeLEDScaling);
				setLED<LedPMixLight>(mixOut / positiveLEDScaling);
				setLED<LedNMix_Sw_Light>(mixOutSW / negativeLEDScaling);
				setLED<LedPMix_Sw_Light>(mixOutSW / positiveLEDScaling);
			}
		}
	}

	void set_samplerate(float sr) override {
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

	static constexpr float maximumShiftInV = 9.5f;
	static constexpr float maximumOutputInV = 10.f;
	static constexpr float minimumOutputInV = -10.f;
	static constexpr float negativeLEDScaling = -8.f;
	static constexpr float positiveLEDScaling = 8.f;

	static float process(float input, float scaleAmount, float shiftAmount) {
		return input * attenueverte(scaleAmount) + attenueverte(shiftAmount) * maximumShiftInV;
	}

	static float attenueverte(float input) {
		return 2.0f * input - 1.0f;
	}
};

} // namespace MetaModule
