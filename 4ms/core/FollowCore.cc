#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Follow_info.hh"
#include "processors/tools/expDecay.h"
#include "processors/tools/schmittTrigger.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Signal In jack; each voice has its own
// follower state. Both outputs carry one channel per voice.
class FollowCore : public PolyCoreProcessor<FollowInfo::NumInJacks, FollowInfo::NumOutJacks> {
	using Info = FollowInfo;
	using ThisCore = FollowCore;

public:
	FollowCore() = default;

	void update(void) override {
		const unsigned nv = num_voices(Info::InputSignal_In);
		auto &in = ins[Info::InputSignal_In];
		auto &env = outs[Info::OutputEnvelope_Out];
		auto &gate = outs[Info::OutputGate_Out];
		env.chans = nv;
		gate.chans = nv;

		if (bypassed) {
			env.values = {};
			gate.values = {};
			return;
		}

		for (unsigned v = 0; v < nv; v++) {
			float rectSignal = in.values[v] / maxOutputVolts;
			if (rectSignal < 0)
				rectSignal *= -1.0f;
			float envOut = slew[v].update(rectSignal);
			wc[v].update(envOut);
			env.values[v] = envOut * maxOutputVolts;
			gate.values[v] = wc[v].output() * maxOutputVolts;
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobThreshold: //threshold
			{
				float topThresh;
				float bottomThresh;
				constexpr float errorAmount = 0.1f;
				topThresh = val + errorAmount;
				if (topThresh > 1.0f)
					topThresh = 1.0f;
				bottomThresh = val - errorAmount;
				if (bottomThresh < 0)
					bottomThresh = 0;
				for (auto &w : wc) {
					w.setHighThreshold(topThresh);
					w.setLowThreshhold(bottomThresh);
				}
			} break;
			case Info::KnobRise: //rise
				for (auto &s : slew)
					s.set_attack_ms(MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 2000.f));
				break;
			case Info::KnobFall: //fall
				for (auto &s : slew)
					s.set_decay_ms(MathTools::map_value(val, 0.0f, 1.0f, 1.0f, 2000.0f));
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobThreshold: //threshold
				return wc[0].getLowThreshold();
			case Info::KnobRise: //rise
				return MathTools::map_value(slew[0].attack_ms(), 1.f, 2000.f, 0.f, 1.f);
			case Info::KnobFall: //fall
				return MathTools::map_value(slew[0].decay_ms(), 1.f, 2000.f, 0.f, 1.f);
		}
		return 0;
	}

	void set_samplerate(float sr) override {
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
	std::array<SchmittTrigger, MaxVoices> wc{};
	std::array<ExpDecay, MaxVoices> slew{};

	static constexpr float maxOutputVolts = 8.0f;
};

} // namespace MetaModule
