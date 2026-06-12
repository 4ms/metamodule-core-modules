#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/Gate_info.hh"

#include "processors/tools/delayLine.h"
#include "processors/tools/schmittTrigger.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Gate In jack; each voice has its own
// delay line and gate timer. The Length/Delay CVs' highest channels feed all
// upper voices.
class GateCore : public PolyCoreProcessor<GateInfo::NumInJacks, GateInfo::NumOutJacks> {
	using Info = GateInfo;
	using ThisCore = GateCore;

public:
	GateCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputGate_In);
		auto &in = ins[Info::InputGate_In];
		auto &out = outs[Info::OutputGate_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = {};
			return;
		}

		auto &lengthIn = ins[Info::InputLength_Cv];
		auto &delayIn = ins[Info::InputDelay_Cv];

		for (unsigned v = 0; v < nv; v++) {
			bool lastGate = currentGate[v];
			currentGate[v] = wc[v].output();
			if (currentGate[v] && !lastGate) {
				float delayCV = delayIn.or_last(v) / CvRangeVolts;
				float lengthCV = lengthIn.or_last(v) / CvRangeVolts;
				float finalDelay =
					MathTools::map_value(MathTools::constrain(delayCV + delayTime, 0.0f, 1.0f), 0.0f, 1.0f, 0.0f, 1000.0f) /
					1000.0f * sampleRate;
				finalLength[v] = MathTools::map_value(
									 MathTools::constrain(lengthCV + gateLength, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f, 1000.0f) /
								 1000.0f * sampleRate;

				del[v].set_delay_samples(finalDelay);

				sinceGate[v] = 0;
			}

			wc[v].update(del[v].update(in.values[v]));

			out.values[v] = (sinceGate[v] < finalLength[v]) ? MaxOutputVolts : 0.f;
			sinceGate[v]++;
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobLength:
				gateLength = val;
				break;
			case Info::KnobDelay:
				delayTime = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobLength:
				return gateLength;
			case Info::KnobDelay:
				return delayTime;
		}
		return 0;
	}

	void set_samplerate(float sr) override {
		sampleRate = sr;
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
	float sampleRate = 48000;
	std::array<bool, MaxVoices> currentGate{};
	std::array<unsigned long, MaxVoices> sinceGate{};
	std::array<float, MaxVoices> finalLength{10, 10, 10, 10};

	float gateLength = 10;
	float delayTime = 0;

	std::array<DelayLine<96000>, MaxVoices> del{};
	std::array<SchmittTrigger, MaxVoices> wc{};
};

} // namespace MetaModule
