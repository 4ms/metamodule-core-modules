#include "CoreModules/register_module.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Atvert2_info.hh"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: each of the two attenuverter channels is independently poly,
// with its voice count following its own input jack.
class Atvert2Core : public PolyCoreProcessor<Atvert2Info::NumInJacks, Atvert2Info::NumOutJacks> {
	using Info = Atvert2Info;
	using ThisCore = Atvert2Core;

public:
	Atvert2Core() = default;

	void update() override {
		auto &in1 = ins[Info::InputCh__1_In];
		auto &in2 = ins[Info::InputCh__2_In];
		auto &out1 = outs[Info::OutputCh__1_Out];
		auto &out2 = outs[Info::OutputCh__2_Out];
		out1.chans = num_voices(Info::InputCh__1_In);
		out2.chans = num_voices(Info::InputCh__2_In);

		if (bypassed) {
			out1.values = in1.values;
			out2.values = in2.values;
			return;
		}

		if (in1.is_patched()) {
			for (unsigned v = 0; v < MaxVoices; v++)
				out1.values[v] = in1.values[v] * level1;
		} else {
			out1.values[0] = defaultVoltage * level1;
		}

		if (in2.is_patched()) {
			for (unsigned v = 0; v < MaxVoices; v++)
				out2.values[v] = in2.values[v] * level2;
		} else {
			out2.values[0] = defaultVoltage * level2;
		}
	}

	void set_param(int param_id, float val) override {
		float bipolarKnob = MathTools::map_value(val, 0.0f, 1.0f, -1.0f, 1.0f);
		switch (param_id) {
			case Info::KnobCh__1:
				level1 = bipolarKnob;
				break;
			case Info::KnobCh__2:
				level2 = bipolarKnob;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobCh__1:
				return level1 / 2.f + 0.5f;
			case Info::KnobCh__2:
				return level2 / 2.f + 0.5f;
		}
		return 0;
	}

	void set_samplerate(const float sr) override {
	}

	static inline bool was_registered = register_module<ThisCore, Info>("4msCompany");

private:
	float level1 = 0;
	float level2 = 0;

	static constexpr float defaultVoltage = 5.f;
};

} // namespace MetaModule
