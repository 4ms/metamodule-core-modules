#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Octave_info.hh"
#include <cmath>

namespace MetaModule
{

class OctaveCore : public CoreProcessor {
	using Info = OctaveInfo;
	using ThisCore = OctaveCore;

public:
	OctaveCore() = default;

	void update() override {
		auto octave = std::round(octaveOffset + cvInput);
		voltOutput = voltInput + octave;
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

	void set_input(int input_id, float val) override {
		if (input_id == Info::InputInput)
			voltInput = val;

		if (input_id == Info::InputCv)
			cvInput = val; //Note: volts!
	}

	float get_output(int output_id) const override {
		return output_id == Info::OutputOut ? voltOutput : 0.f;
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
	float cvInput = 0;
	float voltInput = 0;
	float voltOutput = 0;

	static constexpr float KnobOctaveRange = 8.f;
};

} // namespace MetaModule
