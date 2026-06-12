#include "CoreModules/moduleFactory.hh"
#include "helpers/EdgeDetector.h"
#include "helpers/FlipFlop.h"
#include "helpers/poly_core_processor.hh"
#include "info/SH_info.hh"

namespace MetaModule
{

// Polyphonic: each of the two sample-and-hold sides is independently poly,
// with its voice count following the sampled-value jack ("Clock In"). The
// trigger jack ("Sample In") maps per voice, its highest channel feeding all
// upper voices.
class SHCore : public PolyCoreProcessor<SHInfo::NumInJacks, SHInfo::NumOutJacks> {
	using Info = SHInfo;
	using ThisCore = SHCore;

public:
	SHCore() = default;

	void update() override {
		if (bypassed) {
			outs[0].values = {};
			outs[1].values = {};
			return;
		}

		update_side(0, Info::InputCh__1_Clock_In, Info::InputCh__1_Sample_In);
		update_side(1, Info::InputCh__2_Clock_In, Info::InputCh__2_Sample_In);
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

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	void update_side(unsigned side, unsigned value_jack, unsigned trig_jack) {
		const unsigned nv = num_voices(value_jack);
		auto &value = ins[value_jack];
		auto &trig_in = ins[trig_jack];
		auto &out = outs[side];
		out.chans = nv;

		for (unsigned v = 0; v < nv; v++) {
			unsigned idx = side * MaxVoices + v;
			if (trig[idx](triggerDetector[idx](trig_in.or_last(v))))
				held[idx] = value.values[v];
			out.values[v] = held[idx];
		}
	}

	std::array<float, 2 * MaxVoices> held{};

	static_assert(MaxVoices == 4, "FlipFlop initializer below assumes 4 voices per side");
	std::array<FlipFlop, 2 * MaxVoices> triggerDetector{{{0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f},
														 {0.1f, 0.2f}}};
	std::array<EdgeDetector, 2 * MaxVoices> trig{};
};

} // namespace MetaModule
