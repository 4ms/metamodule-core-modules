#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Octave_info.hh"
#include <cmath>

namespace MetaModule
{

// Polyphonic: voice count follows the Input jack; the CV input's highest
// channel feeds all upper voices.
class OctaveCore : public PolyCoreProcessor<OctaveInfo::NumInJacks, OctaveInfo::NumOutJacks> {
	using Info = OctaveInfo;
	using ThisCore = OctaveCore;

public:
	OctaveCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputInput);
		auto &in = ins[Info::InputInput];
		auto &out = outs[Info::OutputOut];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		alignas(16) std::array<float, MaxVoices> cv;
		ins[Info::InputCv].expand(cv); //Note: volts!

		for (unsigned v = 0; v < MaxVoices; v++)
			out.values[v] = in.values[v] + std::round(octaveOffset + cv[v]);
	}

	void set_param(int param_id, float val) override {
		if (param_id == Info::KnobOctave)
			// 0..1 => -0.5..0.5 => -4..4
			octaveOffset = (val - 0.5f) * KnobOctaveRange;
	}

	float get_param(int param_id) const override {
		if (param_id == Info::KnobOctave)
			// -4..4 => -4 -3 -2..2 3 4 => -0.5 .. 0.5 => 0..1
			return std::round(octaveOffset) / KnobOctaveRange + .5f;
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
	float octaveOffset = 0;

	static constexpr float KnobOctaveRange = 8.f;
};

} // namespace MetaModule
