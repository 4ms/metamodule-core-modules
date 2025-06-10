#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/KPLS_info.hh"

#include "processors/envelope.h"
#include "processors/karplus.h"
#include "util/math.hh"

namespace MetaModule
{

class KPLSCore : public CoreProcessor {
	using Info = KPLSInfo;
	using ThisCore = KPLSCore;

public:
	KPLSCore() {
		e.set_sustain(0.0f);
		e.set_envelope_time(0, 0.1);
		e.set_envelope_time(1, 0);
		e.set_envelope_time(2, 30);
		e.set_decay_curve(0);
		e.set_release_curve(0.5f);
		e.set_sustain(0.1f);
		e.set_envelope_time(3, 200);
		e.sustainEnable = 0;
	}

	void update() override {
		auto noiseOut = MathTools::randomNumber(-1.0f, 1.0f);
		auto noiseBurst = noiseOut * e.update(gateInput);
		k.set_frequency(basePitch * exp5Table.interp(MathTools::constrain(pitchInput, 0.0f, 1.0f)));
		karpOut = k.update(noiseBurst);
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobPitch:
				basePitch = MathTools::map_value(val, 0.0f, 1.0f, 20.0f, 400.0f);
				break;
			case Info::KnobDecay:
				k.set_decay(val);
				e.set_sustain(MathTools::map_value(val, 0.0f, 1.0f, 0.0f, 0.2f));
				e.set_envelope_time(3, MathTools::map_value(val, 0.0f, 1.0f, 200.0f, 1000.0f));
				break;
			case Info::KnobSpread:
				k.set_spread(val);
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobPitch:
				return MathTools::map_value(basePitch, 20.f, 400.f, 0.f, 1.f);
			case Info::KnobDecay:
				return k.get_decay();
			case Info::KnobSpread:
				return k.get_spread();
		}
		return 0;
	}

	void set_input(int input_id, float val) override {
		val = val / CvRangeVolts;

		switch (input_id) {
			case Info::InputTrigger_In:
				gateInput = val;
				break;
			case Info::InputV_Oct_In:
				pitchInput = val;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputAudio_Out)
			return karpOut * MaxOutputVolts;
		return 0.f;
	}

	void mark_all_inputs_unpatched() override {
		mark_input_unpatched(Info::InputTrigger_In);
		mark_input_unpatched(Info::InputV_Oct_In);
	}

	void mark_input_unpatched(int input_id) override {
		switch (input_id) {
			case Info::InputTrigger_In:
				gateInput = 0;
				break;
			case Info::InputV_Oct_In:
				pitchInput = 0;
				break;
		}
	}

	void set_samplerate(float sr) override {
		k.set_samplerate(sr);
		e.set_samplerate(sr);
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
	float karpOut = 0;
	float gateInput = 0;
	float pitchInput = 0;
	float basePitch = 20;

	Karplus k;
	Envelope e;
};

} // namespace MetaModule
