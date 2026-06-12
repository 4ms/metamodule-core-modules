#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Pan_info.hh"

#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; both outputs carry one
// channel per voice. The Pan CV's highest channel feeds all upper voices.
class PanCore : public PolyCoreProcessor<PanInfo::NumInJacks, PanInfo::NumOutJacks> {
	using Info = PanInfo;
	using ThisCore = PanCore;

public:
	PanCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &left = outs[Info::OutputCh__1_Out];
		auto &right = outs[Info::OutputCh__2_Out];
		left.chans = nv;
		right.chans = nv;

		if (bypassed) {
			left.values = in.values;
			right.values = in.values;
			return;
		}

		alignas(16) std::array<float, MaxVoices> panCV;
		ins[Info::InputPan_Cv_In].expand(panCV, 1.0f / CvRangeVolts);

		MM_NO_LOOP_DEPS
		for (unsigned v = 0; v < MaxVoices; v++) {
			float finalPan = MathTools::constrain(panPosition + panCV[v], 0.0f, 1.0f);
			left.values[v] = in.values[v] * (1.0f - finalPan);
			right.values[v] = in.values[v] * finalPan;
		}
	}

	void set_param(int param_id, float val) override {
		if (param_id == Info::KnobPan)
			panPosition = val;
	}

	float get_param(int param_id) const override {
		if (param_id == Info::KnobPan)
			return panPosition;
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
	float panPosition = 0;
};

} // namespace MetaModule
