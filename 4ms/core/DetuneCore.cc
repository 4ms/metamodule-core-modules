#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Detune_info.hh"
#include "processors/pitchShift.h"
#include "util/math.hh"

namespace MetaModule
{

class InterpRandomGenerator {
public:
	float frequency = 1;

	float update() {
		phaccu += frequency / sampleRate;
		if (phaccu >= 1.0f) {
			phaccu -= 1.0f;
			lastValue = currentValue;
			currentValue = MathTools::randomNumber(-1.0f, 1.0f);
		}

		outputValue = MathTools::interpolate(lastValue, currentValue, phaccu);
		return (outputValue);
	}

	void set_samplerate(float sr) {
		sampleRate = sr;
	}

private:
	float sampleRate = 48000.f;
	float currentValue = 0;
	float lastValue = 0;
	float phaccu = 0;
	float outputValue = 0;
};

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// pitch shifter. The wow/flutter modulators are shared by all voices (one
// "tape machine"), but the Detune CV scales the depths per voice, with its
// highest channel feeding all upper voices.
class DetuneCore : public PolyCoreProcessor<DetuneInfo::NumInJacks, DetuneInfo::NumOutJacks> {
	using Info = DetuneInfo;
	using ThisCore = DetuneCore;

public:
	DetuneCore() {
		for (auto &shifter : p) {
			shifter.mix = 1.0f;
			shifter.windowSize = 240;
		}
	}

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &out = outs[Info::OutputAudio_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		auto &cv = ins[Info::InputDetune_Cv_In];
		const bool cvConnected = cv.is_patched();

		const float wowOut = wowGen.update();
		const float flutterOut = flutterGen.update();

		for (unsigned v = 0; v < nv; v++) {
			float finalWow;
			float finalFlutter;
			if (cvConnected == false) {
				finalWow = wowDepth;
				finalFlutter = flutterDepth;
			} else {
				float cvAmount = cv.or_last(v) / CvRangeVolts;
				finalWow = MathTools::constrain(wowDepth + cvAmount, 0.0f, 1.0f);
				finalFlutter = MathTools::constrain(flutterDepth + cvAmount, 0.0f, 1.0f);
			}
			float addWow = wowOut * (finalWow * finalWow);
			float addFlutter = flutterOut * (finalFlutter * finalFlutter);
			p[v].shiftAmount = addWow + addFlutter;
			out.values[v] = p[v].update(in.values[v]);
		}
	}

	void set_param(const int param_id, const float val) override {
		switch (param_id) {
			case Info::KnobWow_Speed:
				wowGen.frequency = MathTools::map_value(val, 0.0f, 1.0f, 0.1f, 5.0f);
				break;
			case Info::KnobWow_Depth:
				wowDepth = val;
				break;
			case Info::KnobFlutter_Speed:
				flutterGen.frequency = MathTools::map_value(val, 0.0f, 1.0f, 5.0f, 30.0f);
				break;
			case Info::KnobFlutter_Depth:
				flutterDepth = val;
				break;
		}
	}

	float get_param(const int param_id) const override {
		switch (param_id) {
			case Info::KnobWow_Speed:
				return MathTools::map_value(wowGen.frequency, .1f, 5.f, 0.f, 1.f);
			case Info::KnobWow_Depth:
				return wowDepth;
			case Info::KnobFlutter_Speed:
				return MathTools::map_value(flutterGen.frequency, 5.0f, 30.0f, 0.f, 1.f);
			case Info::KnobFlutter_Depth:
				return flutterDepth;
		}
		return 0;
	}

	void set_samplerate(const float sr) override {
		for (auto &shifter : p)
			shifter.setSampleRate(sr);
		flutterGen.set_samplerate(sr);
		wowGen.set_samplerate(sr);
	}

	float get_led_brightness(const int led_id) const override {
		return 0.f;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	std::array<PitchShift<9600>, MaxVoices> p{};
	float flutterDepth = 0;
	float wowDepth = 0;

	InterpRandomGenerator flutterGen;
	InterpRandomGenerator wowGen;
};

} // namespace MetaModule
