#include "CoreModules/register_module.hh"
#include "helpers/poly_core_processor.hh"
#include "info/BPF_info.hh"
#include "processors/bpf.h"
#include "processors/oberheimBPF.h"
#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// filter state. The Cutoff CV's highest channel feeds all upper voices.
class BPFCore : public PolyCoreProcessor<BPFInfo::NumInJacks, BPFInfo::NumOutJacks> {
	using Info = BPFInfo;
	using ThisCore = BPFCore;

public:
	BPFCore() = default;

	void update() override {
		const unsigned nv = num_voices(Info::InputAudio_In);
		auto &in = ins[Info::InputAudio_In];
		auto &out = outs[Info::OutputBandpass_Out];
		out.chans = nv;

		if (bypassed) {
			out.values = in.values;
			return;
		}

		auto &cutoff = ins[Info::InputCutoff_Cv_In];

		for (unsigned v = 0; v < nv; v++) {
			float cutoffCV = cutoff.or_last(v) / CvRangeVolts;
			float filterFreq = 523.25f * setPitchMultiple(constrain(cutoffCV + cutoffOffset, -1.0f, 1.0f));
			if (mode == 0) {
				bpf[v].q = map_value(filterQ, 0.0f, 1.0f, 1.0f, 20.0f);
				bpf[v].cutoff.setValue(filterFreq);
				out.values[v] = bpf[v].update(in.values[v]);
			} else if (mode == 1) {
				ober[v].q = map_value(filterQ, 0.0f, 1.0f, 1.0f, 20.0f);
				ober[v].cutoff.setValue(audioFreqToNorm(filterFreq));
				out.values[v] = ober[v].update(in.values[v]);
			}
		}
	}

	void set_param(int const param_id, const float val) override {
		switch (param_id) {
			case Info::KnobCutoff:
				cutoffOffset = map_value(val, 0.0f, 1.0f, -1.0f, 1.0f);
				break;
			case Info::KnobQ:
				filterQ = val;
				break;
			case (static_cast<unsigned>(Info::SwitchMode) + static_cast<unsigned>(Info::NumKnobs)):
				mode = val > 0.5f ? 1 : 0;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobCutoff:
				return map_value(cutoffOffset, -1.f, 1.f, 0.f, 1.f);
			case Info::KnobQ:
				return filterQ;
			case (static_cast<unsigned>(Info::SwitchMode) + static_cast<unsigned>(Info::NumKnobs)):
				return mode;
		}
		return 0;
	}

	void set_samplerate(const float sr) override {
		for (auto &f : bpf)
			f.sampleRate.setValue(sr);
	}

	float get_led_brightness(const int led_id) const override {
		return mode;
	}

	static inline bool was_registered = register_module<ThisCore, Info>("4msCompany");

private:
	int mode = 0;
	float filterQ = 1;
	std::array<BandPassFilter, MaxVoices> bpf{};
	std::array<OberBPF, MaxVoices> ober{};
	float cutoffOffset = 0;
};

} // namespace MetaModule
