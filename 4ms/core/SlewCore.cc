#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Slew_info.hh"
#include "processors/tools/expDecay.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Signal In jack; each voice has its own
// slew state, sharing the Rise/Fall settings.
class SlewCore : public PolyCoreProcessor<SlewInfo::NumInJacks, SlewInfo::NumOutJacks> {
	using Info = SlewInfo;
	using ThisCore = SlewCore;

public:
	SlewCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputSignal_In);
		auto &in = ins[Info::InputSignal_In];
		auto &out = outs[Info::OutputSlewed_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		for (unsigned v = 0; v < nv; v++)
			out.values[v] = slew[v].update(in.values[v]);
	}

	void set_param(int param_id, float val) override {
		if (val < 0)
			val = 0.f;

		switch (param_id) {
			case Info::KnobRise:
				for (auto &s : slew)
					s.set_attack_ms(MathTools::map_value(val, 0.0f, CvRangeVolts, 1.0f, 2000.0f));
				break;
			case Info::KnobFall:
				for (auto &s : slew)
					s.set_decay_ms(MathTools::map_value(val, 0.0f, CvRangeVolts, 1.0f, 2000.0f));
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobRise:
				return MathTools::map_value(slew[0].attack_ms(), 1.f, 2000.f, .0f, CvRangeVolts);
			case Info::KnobFall:
				return MathTools::map_value(slew[0].decay_ms(), 1.f, 2000.f, .0f, CvRangeVolts);
		}
		return 0;
	}

	void set_samplerate(float sr) override {
		for (auto &s : slew)
			s.set_samplerate(sr);
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
	std::array<ExpDecay, MaxVoices> slew{};
};

} // namespace MetaModule
