#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
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

class DetuneCore : public CoreProcessor {
	using Info = DetuneInfo;
	using ThisCore = DetuneCore;

public:
	DetuneCore() {
		p.mix = 1.0f;
		p.windowSize = 240;
	}

	void update() override {
		if (bypassed) {
			signalOutput = signalInput;
			return;
		}

		float addWow = 0;
		float addFlutter = 0;
		float finalWow = 0;
		float finalFlutter = 0;
		if (detuneCvConnected == false) {
			finalWow = wowDepth;
			finalFlutter = flutterDepth;
		} else {
			finalWow = MathTools::constrain(wowDepth + cvAmount, 0.0f, 1.0f);
			finalFlutter = MathTools::constrain(flutterDepth + cvAmount, 0.0f, 1.0f);
		}
		addWow = wowGen.update() * (finalWow * finalWow);
		addFlutter = flutterGen.update() * (finalFlutter * finalFlutter);
		p.shiftAmount = addWow + addFlutter;
		signalOutput = p.update(signalInput);
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
		p.setSampleRate(sr);
		flutterGen.set_samplerate(sr);
		wowGen.set_samplerate(sr);
	}

	void set_input(const int input_id, const float val) override {
		switch (input_id) {
			case Info::InputAudio_In:
				signalInput = val;
				break;
			case Info::InputDetune_Cv_In:
				cvAmount = val / CvRangeVolts;
				break;
		}
	}

	float get_output(const int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return signalOutput;
		return 0.f;
	}

	void mark_input_unpatched(const int input_id) override {
		if (input_id == Info::InputDetune_Cv_In)
			detuneCvConnected = false;
	}

	void mark_input_patched(const int input_id) override {
		if (input_id == Info::InputDetune_Cv_In)
			detuneCvConnected = true;
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
	PitchShift<9600> p;
	float flutterDepth = 0;
	float wowDepth = 0;

	float signalInput = 0;
	float signalOutput = 0;

	bool detuneCvConnected = false;
	float cvAmount = 0;

	InterpRandomGenerator flutterGen;
	InterpRandomGenerator wowGen;
};

} // namespace MetaModule
