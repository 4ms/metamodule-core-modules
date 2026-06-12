#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Switch41_info.hh"
#include "processors/tools/clockPhase.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: the output channel count is the highest channel count of the
// four signal inputs (each input's highest channel feeds its upper voices).
// The Clock, Reset, and CV controls are mono and switch all voices together.
class Switch41Core : public PolyCoreProcessor<Switch41Info::NumInJacks, Switch41Info::NumOutJacks> {
	using Info = Switch41Info;
	using ThisCore = Switch41Core;

public:
	Switch41Core() = default;

	void update() override {
		if (bypassed)
			return;

		cp.updateClock(ins[Info::InputClock].values[0]);
		cp.updateReset(ins[Info::InputReset].values[0]);
		cp.update();
		stepNum = cp.getCount() % 4;

		float cvInput = MathTools::constrain(ins[Info::InputCv].values[0] / CvRangeVolts, 0.0f, 1.0f);
		float position = cvInput * 3.0f;
		float fade = position - (int)position;

		switch ((int)position) {
			case 0:
				scanLevels[0] = 1.0f - fade;
				scanLevels[1] = fade;
				scanLevels[2] = 0;
				scanLevels[3] = 0;
				break;
			case 1:
				scanLevels[1] = 1.0f - fade;
				scanLevels[2] = fade;
				scanLevels[0] = 0;
				scanLevels[3] = 0;
				break;
			case 2:
				scanLevels[2] = 1.0f - fade;
				scanLevels[3] = fade;
				scanLevels[0] = 0;
				scanLevels[1] = 0;
				break;
			case 3:
				scanLevels[0] = 0;
				scanLevels[1] = 0;
				scanLevels[2] = 0;
				scanLevels[3] = 1.0f;
				break;
		}

		// Output channel count: the widest of the four signal inputs
		unsigned nv = 1;
		for (unsigned i = 0; i < 4; i++)
			nv = std::max<unsigned>(nv, ins[Info::InputCh__1_In + i].chans);
		nv = std::min(nv, MaxVoices);

		auto &out = outs[Info::OutputOut];
		out.chans = nv;

		for (unsigned v = 0; v < nv; v++) {
			float output = 0.0f;
			if (cvMode) {
				for (unsigned i = 0; i < 4; i++)
					output += ins[Info::InputCh__1_In + i].or_last(v) * scanLevels[i];
			} else {
				output = ins[Info::InputCh__1_In + stepNum].or_last(v);
			}
			out.values[v] = output;
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
	int stepNum = 0;
	float scanLevels[4] = {0, 0, 0, 0};
	bool cvMode = false;
	ClockPhase cp;
};

} // namespace MetaModule
