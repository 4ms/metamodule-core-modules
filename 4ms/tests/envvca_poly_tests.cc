//Hack: include the .cc file (same approach as djembe_tests):
#include "CoreModules/4ms/core/ENVVCACore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

struct EnvvcaUnderTest {
	ENVVCACore m;
	CoreProcessor::PolyPortBuffer trig;
	CoreProcessor::PolyPortBuffer audioIn;
	CoreProcessor::PolyPortBuffer envOut;
	CoreProcessor::PolyPortBuffer eorOut;
	CoreProcessor::PolyPortBuffer audioOut;

	// Param ids in element order:
	enum { RiseRange = 0, FallRange, CycleButton, RiseSlider, FallSlider, EnvLevel, RiseCv, FallCv };

	EnvvcaUnderTest() {
		m.set_samplerate(48000.f);
		m.set_param(RiseRange, 0.5f); // center (Med)
		m.set_param(FallRange, 0.5f);
		m.set_param(RiseSlider, 0.0f); // fastest (~37ms)
		m.set_param(FallSlider, 0.0f);
		m.set_param(EnvLevel, 1.0f);

		trig = m.get_poly_input_buffer(ENVVCAInfo::InputTrigger_In);
		audioIn = m.get_poly_input_buffer(ENVVCAInfo::InputAudio_In);
		envOut = m.get_poly_output_buffer(ENVVCAInfo::OutputEnv_Out);
		eorOut = m.get_poly_output_buffer(ENVVCAInfo::OutputEor_Out);
		audioOut = m.get_poly_output_buffer(ENVVCAInfo::OutputAudio_Out);

		m.mark_output_patched(ENVVCAInfo::OutputEnv_Out);
		m.mark_output_patched(ENVVCAInfo::OutputEor_Out);
		m.mark_output_patched(ENVVCAInfo::OutputAudio_Out);
	}

	void patch_trig(unsigned chans) {
		m.mark_input_patched(ENVVCAInfo::InputTrigger_In);
		*trig.channels = chans;
	}

	void patch_audio_in(unsigned chans, float volts) {
		m.mark_input_patched(ENVVCAInfo::InputAudio_In);
		*audioIn.channels = chans;
		for (unsigned ch = 0; ch < chans; ch++)
			audioIn.voltages[ch] = volts;
	}

	// Run, tracking the max absolute voltage seen per channel on a poly output
	std::array<float, 4> run_peak(unsigned num_samples, CoreProcessor::PolyPortBuffer const &out) {
		std::array<float, 4> peak{};
		for (unsigned i = 0; i < num_samples; i++) {
			m.update();
			for (unsigned ch = 0; ch < *out.channels; ch++)
				peak[ch] = std::max(peak[ch], std::abs(out.voltages[ch]));
		}
		return peak;
	}

	void pulse_trig(unsigned chan) {
		trig.voltages[chan] = 8.f;
		for (unsigned i = 0; i < 100; i++)
			m.update();
		trig.voltages[chan] = 0.f;
	}
};

} // namespace

TEST_CASE("ENVVCA poly: env path and audio path have independent channel counts") {
	EnvvcaUnderTest t;

	// Nothing patched: all outputs mono
	t.m.update();
	CHECK(*t.envOut.channels == 1);
	CHECK(*t.eorOut.channels == 1);
	CHECK(*t.audioOut.channels == 1);

	// Trigger channels drive Env Out and EOR Out only
	t.patch_trig(2);
	t.m.update();
	CHECK(*t.envOut.channels == 2);
	CHECK(*t.eorOut.channels == 2);
	CHECK(*t.audioOut.channels == 1);

	// Audio In channels drive Audio Out only
	t.patch_audio_in(3, 0.f);
	t.m.update();
	CHECK(*t.envOut.channels == 2);
	CHECK(*t.eorOut.channels == 2);
	CHECK(*t.audioOut.channels == 3);

	// Unpatching returns to mono
	t.m.mark_input_unpatched(ENVVCAInfo::InputTrigger_In);
	t.m.mark_input_unpatched(ENVVCAInfo::InputAudio_In);
	t.m.update();
	CHECK(*t.envOut.channels == 1);
	CHECK(*t.audioOut.channels == 1);
}

TEST_CASE("ENVVCA poly: each trigger channel fires its own envelope") {
	EnvvcaUnderTest t;
	t.patch_trig(3);

	t.pulse_trig(1);
	auto peak = t.run_peak(48000, t.envOut);

	CHECK(peak[1] > 3.f);
	CHECK(peak[0] < 0.05f);
	CHECK(peak[2] < 0.05f);

	// EOR fires only on the triggered channel
	t.pulse_trig(2);
	auto eor = t.run_peak(48000, t.eorOut);
	CHECK(eor[2] == doctest::Approx(8.f));
	CHECK(eor[0] == doctest::Approx(0.f));
}

TEST_CASE("ENVVCA poly: highest envelope controls all upper audio channels") {
	EnvvcaUnderTest t;
	t.patch_trig(2);
	t.patch_audio_in(4, 5.f);

	// VCA closed: all audio channels off
	auto quiet = t.run_peak(1000, t.audioOut);
	for (unsigned ch = 0; ch < 4; ch++)
		CHECK(quiet[ch] < 0.05f);

	// Envelope 1 controls audio channels 1, 2, and 3; channel 0 stays closed
	t.pulse_trig(1);
	auto peak = t.run_peak(48000, t.audioOut);
	CHECK(peak[1] > 1.f);
	CHECK(peak[2] > 1.f);
	CHECK(peak[3] > 1.f);
	CHECK(peak[0] < 0.05f);

	// Envelope 0 controls only audio channel 0
	t.pulse_trig(0);
	auto peak0 = t.run_peak(48000, t.audioOut);
	CHECK(peak0[0] > 1.f);
}

TEST_CASE("ENVVCA poly: audio channels follow their own envelope when counts match") {
	EnvvcaUnderTest t;
	t.patch_trig(2);
	t.patch_audio_in(2, 5.f);

	t.pulse_trig(0);
	auto peak = t.run_peak(48000, t.audioOut);
	CHECK(peak[0] > 1.f);
	CHECK(peak[1] < 0.05f);
}

TEST_CASE("ENVVCA poly: mono operation via set_input still works") {
	EnvvcaUnderTest t;

	t.m.mark_input_patched(ENVVCAInfo::InputTrigger_In);
	t.m.mark_input_patched(ENVVCAInfo::InputAudio_In);
	t.m.set_input(ENVVCAInfo::InputAudio_In, 5.f);

	t.m.set_input(ENVVCAInfo::InputTrigger_In, 8.f);
	for (unsigned i = 0; i < 100; i++)
		t.m.update();
	t.m.set_input(ENVVCAInfo::InputTrigger_In, 0.f);

	auto envPeak = t.run_peak(48000, t.envOut);
	CHECK(*t.envOut.channels == 1);
	CHECK(*t.audioOut.channels == 1);
	CHECK(envPeak[0] > 3.f);
	CHECK(t.m.get_output(ENVVCAInfo::OutputEnv_Out) == t.envOut.voltages[0]);
}

TEST_CASE("ENVVCA poly: unpatched outputs keep 0 channels") {
	EnvvcaUnderTest t;

	t.m.mark_output_unpatched(ENVVCAInfo::OutputAudio_Out);
	t.patch_trig(2);
	t.m.update();

	// setChannels() must not resurrect an unpatched output
	CHECK(*t.audioOut.channels == 0);
	CHECK(*t.envOut.channels == 2);
}
