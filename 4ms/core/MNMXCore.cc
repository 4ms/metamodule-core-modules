#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/MNMX_info.hh"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: the output channel count is the wider of the two inputs; each
// input's highest channel feeds its upper voices.
class MNMXCore : public PolyCoreProcessor<MNMXInfo::NumInJacks, MNMXInfo::NumOutJacks> {
	using Info = MNMXInfo;
	using ThisCore = MNMXCore;

public:
	MNMXCore() = default;

	void update() override {
		auto &inA = ins[Info::InputIn_A];
		auto &inB = ins[Info::InputIn_B];
		auto &outMin = outs[Info::OutputMin];
		auto &outMax = outs[Info::OutputMax];

		const unsigned nv = std::clamp<unsigned>(std::max(inA.chans, inB.chans), 1, MaxVoices);
		outMin.chans = nv;
		outMax.chans = nv;

		if (bypassed) {
			outMin.values = {};
			outMax.values = {};
			return;
		}

		for (unsigned v = 0; v < nv; v++) {
			float a = inA.or_last(v);
			float b = inB.or_last(v);
			outMin.values[v] = MathTools::min<float>(a, b);
			outMax.values[v] = MathTools::max<float>(a, b);
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

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on
};

} // namespace MetaModule
