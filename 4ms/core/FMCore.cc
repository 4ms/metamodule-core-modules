#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/FM_info.hh"

#include "processors/twoOpFMOscillator.h"
#include "util/math.hh"

namespace MetaModule
{

class FMCore : public CoreProcessor {
	using Info = FMInfo;
	using ThisCore = FMCore;

public:
	FMCore() {
		set_samplerate(48000);
	}

	void update(void) override {
		fm.set_frequency(0, basePitch * MathTools::setPitchMultiple(pitchInput));
		if (secondPitchConnected == false) {
			fm.set_frequency(1, basePitch * ratioFine * ratioCoarse);
		} else {
			fm.set_frequency(1, basePitch * MathTools::setPitchMultiple(secondPitchInput));
		}
		totalIndex = MathTools::constrain(indexCV * indexAmount + indexKnob, 0.0f, 1.0f);
		float totalShape = MathTools::constrain(shapeCV * shapeAmount + shapeKnob, 0.0f, 1.0f);
		fm.shape = totalShape;
		fm.modAmount = totalIndex;
		float finalMix = MathTools::constrain(mixCV + mix, 0.0f, 1.0f);
		fm.mix = finalMix;
		mainOutput = fm.update();
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobPitch:
				basePitch = 20.0f * exp5Table.interp(val);
				break;
			case Info::KnobIndex:
				indexKnob = val;
				break;
			case Info::KnobRatio_Coarse:
				ratioCoarse = ratioTable[MathTools::map_value(val, 0.0f, 1.0f, 0, 7)];
				break;
			case Info::KnobRatio_Fine:
				if (val < 0.5f)
					ratioFine = MathTools::map_value(val, 0.0f, 0.5f, 0.5f, 1.0f);
				else
					ratioFine = MathTools::map_value(val, 0.5f, 1.0f, 1.0f, 2.0f);
				break;
			case Info::KnobShape:
				shapeKnob = val;
				break;
			case Info::KnobShape_Cv:
				shapeAmount = val;
				break;
			case Info::KnobIndex_Cv:
				indexAmount = val;
				break;
			case Info::KnobMix:
				mix = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobPitch: {
				auto t = std::lower_bound(exp5Table.data.begin(), exp5Table.data.end(), basePitch / 20.f);
				return std::distance(exp5Table.data.begin(), t) / static_cast<float>(exp5Table.data.size() - 1);
			}
			case Info::KnobIndex:
				return indexKnob;
			case Info::KnobRatio_Coarse: {
				auto t = std::find(ratioTable.begin(), ratioTable.end(), ratioCoarse);
				return std::distance(ratioTable.begin(), t) / static_cast<float>(ratioTable.size() - 1);
			}
			case Info::KnobRatio_Fine:
				if (ratioFine > 1.f)
					return MathTools::map_value(ratioFine, 1.f, 2.f, .5f, 1.f);
				else
					return MathTools::map_value(ratioFine, .5f, 1.f, 0.f, .5f);
			case Info::KnobShape:
				return shapeKnob;
			case Info::KnobShape_Cv:
				return shapeAmount;
			case Info::KnobIndex_Cv:
				return indexAmount;
			case Info::KnobMix:
				return mix;
		}
		return 0;
	}

	void set_input(int input_id, float val) override {
		val = val / cvRangeVolts;
		switch (input_id) {
			case Info::InputIndex_Cv_In:
				indexCV = val;
				break;
			case Info::InputV_Oct_Carrier:
				pitchInput = val;
				break;
			case Info::InputShape_Cv_In:
				shapeCV = val;
				break;
			case Info::InputMix_Cv_In:
				mixCV = val;
				break;
			case Info::InputV_Oct_Modulator:
				secondPitchInput = val;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return mainOutput * maxOutputVolts;
		return 0.f;
	}

	void set_samplerate(float sr) override {
		fm.set_samplerate(sr);
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	void mark_input_unpatched(int input_id) override {
		if (input_id == Info::InputV_Oct_Modulator)
			secondPitchConnected = false;
	}

	void mark_input_patched(int input_id) override {
		if (input_id == Info::InputV_Oct_Modulator)
			secondPitchConnected = true;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	TwoOpFM fm;
	float ratioCoarse = 1;
	float ratioFine = 1;
	float mix = 0;
	float mixCV = 0;
	float basePitch = 0;
	float mainOutput;
	float indexKnob = 0;
	float secondPitchInput = 0;
	bool secondPitchConnected = false;
	float indexCV = 0;
	float shapeKnob = 0;
	float shapeCV = 0;
	float totalIndex = 0;
	float pitchInput = 0;
	float shapeAmount = 0;
	float indexAmount = 0;

	static constexpr float cvRangeVolts = 5.0f;
	static constexpr float maxOutputVolts = 8.0f;

	const std::array<float, 8> ratioTable = {0.125f, 0.25f, 0.5f, 1, 2, 4, 8, 16};
};

} // namespace MetaModule
