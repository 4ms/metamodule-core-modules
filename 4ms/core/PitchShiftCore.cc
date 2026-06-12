#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/PitchShift_info.hh"
#include "processors/pitchShift.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// pitch shifter. The CV inputs' highest channels feed all upper voices.
class PitchShiftCore : public PolyCoreProcessor<PitchShiftInfo::NumInJacks, PitchShiftInfo::NumOutJacks> {
	using Info = PitchShiftInfo;
	using ThisCore = PitchShiftCore;

public:
	PitchShiftCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &out = outs[Info::OutputAudio_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		auto &pitchCv = ins[Info::InputPitch_Cv];
		auto &windowCv = ins[Info::InputWindow_Cv];
		auto &mixCv = ins[Info::InputMix_Cv];

		for (unsigned v = 0; v < nv; v++) {
			float shiftCV = MathTools::constrain(pitchCv.or_last(v), -5.f, +5.f);
			float windowCV = windowCv.or_last(v) / CvRangeVolts;
			float mixCV = mixCv.or_last(v) / CvRangeVolts;

			auto finalWindow = MathTools::constrain(windowOffset + windowCV, 0.0f, 1.0f);
			p[v].windowSize = MathTools::map_value(finalWindow, 0.0f, 1.0f, 20.0f, static_cast<float>(maxWindowSize));
			p[v].shiftAmount = coarseShift + fineShift + shiftCV * 12.f;
			p[v].mix = MathTools::constrain(mixOffset + mixCV, 0.0f, 1.0f);
			out.values[v] = p[v].update(in.values[v]);
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobCoarse:
				coarseShift = MathTools::map_value(val, 0.0f, 1.0f, -12.0f, 12.0f);
				break;
			case Info::KnobFine:
				fineShift = MathTools::map_value(val, 0.0f, 1.0f, -1.0f, 1.0f);
				break;
			case Info::KnobWindow:
				windowOffset = val;
				break;
			case Info::KnobMix:
				mixOffset = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobCoarse:
				return MathTools::map_value(coarseShift, -12.f, 12.f, 0.f, 1.f);
			case Info::KnobFine:
				return MathTools::map_value(fineShift, -1.f, 1.f, 0.f, 1.f);
			case Info::KnobWindow:
				return windowOffset;
			case Info::KnobMix:
				return mixOffset;
		}
		return 0;
	}

	void set_samplerate(float sr) override {
		for (auto &shifter : p)
			shifter.setSampleRate(sr);
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	constexpr static int maxWindowSize = 9600;
	std::array<PitchShift<maxWindowSize>, MaxVoices> p{};

	float coarseShift = 0;
	float fineShift = 0;
	float mixOffset = 0;
	float windowOffset = 100;
};

} // namespace MetaModule
