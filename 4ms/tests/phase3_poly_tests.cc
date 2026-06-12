//Hack: include the .cc files (same approach as djembe_tests):
#include "CoreModules/4ms/core/ComplexEGCore.cc"
#include "CoreModules/4ms/core/DrumCore.cc"
#include "CoreModules/4ms/core/PitchShiftCore.cc"
#include "CoreModules/4ms/core/StMixCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

void patch(CoreProcessor &m, int input_id, CoreProcessor::PolyPortBuffer const &buf, unsigned chans) {
	m.mark_input_patched(input_id);
	*buf.channels = chans;
}

} // namespace

TEST_CASE("ComplexEG poly: per-voice envelopes and stage outputs") {
	ComplexEGCore m;
	m.set_samplerate(48000.f);
	// Element-order params: Attack=0 Decay=1 Release=2 (curves 3-5) Sustain=6 Loop=7 Hold=8
	m.set_param(0, 0.1f);  // fast-ish attack
	m.set_param(6, 0.5f);  // sustain 0.5

	auto gate = m.get_poly_input_buffer(ComplexEGInfo::InputGate_In);
	auto env = m.get_poly_output_buffer(ComplexEGInfo::OutputEnv_Out);
	auto sus = m.get_poly_output_buffer(ComplexEGInfo::OutputSustain_Out);
	m.mark_output_patched(ComplexEGInfo::OutputEnv_Out);
	m.mark_output_patched(ComplexEGInfo::OutputSustain_Out);

	patch(m, ComplexEGInfo::InputGate_In, gate, 2);

	// Let the power-on envelope cycle finish
	for (int i = 0; i < 96000; i++)
		m.update();

	CHECK(*env.channels == 2);
	CHECK(env.voltages[0] == doctest::Approx(0.f));
	CHECK(env.voltages[1] == doctest::Approx(0.f));

	// Gate voice 1: it reaches sustain, voice 0 stays idle
	gate.voltages[1] = 8.f;
	for (int i = 0; i < 48000; i++)
		m.update();

	CHECK(env.voltages[0] == doctest::Approx(0.f));
	CHECK(env.voltages[1] > 3.f);
	CHECK(sus.voltages[1] == doctest::Approx(8.f));
	CHECK(sus.voltages[0] == doctest::Approx(0.f));
}

TEST_CASE("PitchShift poly: per-voice shifting, output follows input") {
	PitchShiftCore m;
	m.set_samplerate(48000.f);
	m.set_param(PitchShiftInfo::KnobCoarse, 1.0f); // +12 semitones
	m.set_param(PitchShiftInfo::KnobMix, 1.0f);	   // wet
	m.set_param(PitchShiftInfo::KnobWindow, 0.5f);

	auto in = m.get_poly_input_buffer(PitchShiftInfo::InputAudio_In);
	auto out = m.get_poly_output_buffer(PitchShiftInfo::OutputAudio_Out);

	patch(m, PitchShiftInfo::InputAudio_In, in, 3);

	float energy[3]{};
	for (int i = 0; i < 9600; i++) {
		in.voltages[0] = 0.f;
		in.voltages[1] = (i / 32) % 2 ? 5.f : -5.f;
		in.voltages[2] = 0.f;
		m.update();
		for (int ch = 0; ch < 3; ch++)
			energy[ch] += std::abs(out.voltages[ch]);
	}
	CHECK(*out.channels == 3);
	CHECK(energy[1] > 100.f);
	CHECK(energy[0] == doctest::Approx(0.f));
	CHECK(energy[2] == doctest::Approx(0.f));
}

TEST_CASE("StMix poly: output is widest input, right normals to left") {
	StMixCore m;
	auto in1L = m.get_poly_input_buffer(StMixInfo::InputCh__1_Left_In);
	auto in1R = m.get_poly_input_buffer(StMixInfo::InputCh__1_Right_In);
	auto in2L = m.get_poly_input_buffer(StMixInfo::InputCh__2_Left_In);
	auto outL = m.get_poly_output_buffer(StMixInfo::OutputLeft_Out);
	auto outR = m.get_poly_output_buffer(StMixInfo::OutputRight_Out);

	// Ch1 Left poly 2ch; Right unpatched (normals to Left)
	patch(m, StMixInfo::InputCh__1_Left_In, in1L, 2);
	in1L.voltages[0] = 1.f;
	in1L.voltages[1] = 2.f;

	m.update();
	CHECK(*outL.channels == 2);
	CHECK(*outR.channels == 2);
	CHECK(outL.voltages[0] == doctest::Approx(1.f));
	CHECK(outL.voltages[1] == doctest::Approx(2.f));
	CHECK(outR.voltages[0] == doctest::Approx(1.f));
	CHECK(outR.voltages[1] == doctest::Approx(2.f));

	// Patching Ch1 Right (mono) overrides the normalling on all voices
	patch(m, StMixInfo::InputCh__1_Right_In, in1R, 1);
	in1R.voltages[0] = 9.f;
	m.update();
	CHECK(outR.voltages[0] == doctest::Approx(9.f));
	CHECK(outR.voltages[1] == doctest::Approx(9.f));
	CHECK(outL.voltages[1] == doctest::Approx(2.f));

	// A mono input on Ch2 sums into all voices (copy-highest)
	patch(m, StMixInfo::InputCh__2_Left_In, in2L, 1);
	in2L.voltages[0] = 0.5f;
	m.update();
	CHECK(outL.voltages[0] == doctest::Approx(1.5f));
	CHECK(outL.voltages[1] == doctest::Approx(2.5f));
}

TEST_CASE("Drum poly: each trigger channel fires its own drum voice") {
	DrumCore m;
	m.set_samplerate(48000.f);
	m.set_param(DrumInfo::KnobPitch, 0.3f);
	m.set_param(DrumInfo::KnobNoise_Blend, 0.5f);

	auto trig = m.get_poly_input_buffer(DrumInfo::InputTrigger_In);
	auto audio = m.get_poly_output_buffer(DrumInfo::OutputAudio_Out);
	auto toneEnv = m.get_poly_output_buffer(DrumInfo::OutputTone_Env__Out);

	patch(m, DrumInfo::InputTrigger_In, trig, 2);

	// Let the power-on envelope cycles finish
	for (int i = 0; i < 96000; i++)
		m.update();

	// Trigger voice 1 only
	trig.voltages[1] = 8.f;
	for (int i = 0; i < 100; i++)
		m.update();
	trig.voltages[1] = 0.f;

	float energy[2]{};
	float tenvPeak[2]{};
	for (int i = 0; i < 24000; i++) {
		m.update();
		for (int ch = 0; ch < 2; ch++) {
			energy[ch] += std::abs(audio.voltages[ch]);
			tenvPeak[ch] = std::max(tenvPeak[ch], toneEnv.voltages[ch]);
		}
	}
	CHECK(*audio.channels == 2);
	CHECK(*toneEnv.channels == 2);
	CHECK(energy[1] > 10.f);
	CHECK(energy[0] < energy[1] * 0.01f);
	CHECK(tenvPeak[1] > 1.f);
	CHECK(tenvPeak[0] < 0.05f);
}
