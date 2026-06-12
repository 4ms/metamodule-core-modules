#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Switch14_info.hh"
#include "processors/tools/clockPhase.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: all four outputs follow the Signal In jack's channel count.
// The Clock, Reset, and CV controls are mono and switch all voices together.
class Switch14Core : public PolyCoreProcessor<Switch14Info::NumInJacks, Switch14Info::NumOutJacks> {
	using Info = Switch14Info;
	using ThisCore = Switch14Core;

public:
	Switch14Core() = default;

	void update() override {
		if (bypassed)
			return;

		cp.updateClock(ins[Info::InputClock].values[0] / CvRangeVolts);
		cp.updateReset(ins[Info::InputReset].values[0] / CvRangeVolts);
		cp.update();
		stepNum = cp.getCount() % NumThrows;

		if (cvMode) {
			float cvSignal = MathTools::constrain(ins[Info::InputCv].values[0] / CvRangeVolts, 0.0f, 1.0f);
			float position = cvSignal * 3.0f;
			float fade = position - (int)position;

			switch ((int)position) {
				case 0:
					panSignals[0] = 1.0f - fade;
					panSignals[1] = fade;
					panSignals[2] = 0;
					panSignals[3] = 0;
					break;
				case 1:
					panSignals[0] = 0;
					panSignals[1] = 1.0f - fade;
					panSignals[2] = fade;
					panSignals[3] = 0;
					break;
				case 2:
					panSignals[0] = 0;
					panSignals[1] = 0;
					panSignals[2] = 1.0f - fade;
					panSignals[3] = fade;
					break;
				case 3:
					panSignals[0] = 0;
					panSignals[1] = 0;
					panSignals[2] = 0;
					panSignals[3] = 1.0f;
					break;
			}
		}

		// Output jacks must be sequential
		// or else our logic doesn't work:
		static_assert(Info::OutputCh__1_Out + 1 == Info::OutputCh__2_Out);
		static_assert(Info::OutputCh__2_Out + 1 == Info::OutputCh__3_Out);
		static_assert(Info::OutputCh__3_Out + 1 == Info::OutputCh__4_Out);

		const unsigned nv = num_voices(Info::InputSignal_In);
		auto &in = ins[Info::InputSignal_In];

		for (unsigned k = 0; k < NumThrows; k++) {
			auto &out = outs[Info::OutputCh__1_Out + k];
			out.chans = nv;
			for (unsigned v = 0; v < nv; v++) {
				if (cvMode)
					out.values[v] = panSignals[k] * in.values[v];
				else
					out.values[v] = (k == (unsigned)stepNum) ? in.values[v] : 0.f;
			}
		}
	}

	void set_param(int param_id, float val) override {
	}

	float get_param(int param_id) const override {
		return 0;
	}

	void set_samplerate(float sr) override {
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	void mark_input_unpatched(const int input_id) override {
		PolyCoreProcessor::mark_input_unpatched(input_id);
		if (input_id == Info::InputCv)
			cvMode = false;
	}

	void mark_input_patched(const int input_id) override {
		PolyCoreProcessor::mark_input_patched(input_id);
		if (input_id == Info::InputCv)
			cvMode = true;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	static constexpr size_t NumThrows = 4;
	ClockPhase cp;
	float panSignals[NumThrows]{};
	bool cvMode = false;
	int stepNum = 0;
};

} // namespace MetaModule
