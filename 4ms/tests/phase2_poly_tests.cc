//Hack: include the .cc files (same approach as djembe_tests):
#include "CoreModules/4ms/core/DetuneCore.cc"
#include "CoreModules/4ms/core/FollowCore.cc"
#include "CoreModules/4ms/core/GateCore.cc"
#include "CoreModules/4ms/core/LPGCore.cc"
#include "CoreModules/4ms/core/SHCore.cc"
#include "CoreModules/4ms/core/Switch14Core.cc"
#include "CoreModules/4ms/core/Switch41Core.cc"

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

TEST_CASE("Follow poly: per-voice envelope and gate") {
	FollowCore m;
	m.set_param(FollowInfo::KnobThreshold, 0.2f);
	m.set_param(FollowInfo::KnobRise, 0.0f); // fast
	m.set_param(FollowInfo::KnobFall, 0.5f);

	auto in = m.get_poly_input_buffer(FollowInfo::InputSignal_In);
	auto env = m.get_poly_output_buffer(FollowInfo::OutputEnvelope_Out);
	auto gate = m.get_poly_output_buffer(FollowInfo::OutputGate_Out);

	patch(m, FollowInfo::InputSignal_In, in, 2);
	in.voltages[0] = 0.f;
	in.voltages[1] = 8.f;

	for (int i = 0; i < 4800; i++)
		m.update();

	CHECK(*env.channels == 2);
	CHECK(*gate.channels == 2);
	CHECK(env.voltages[0] == doctest::Approx(0.f));
	CHECK(env.voltages[1] > 4.f);
	CHECK(gate.voltages[0] == doctest::Approx(0.f));
	CHECK(gate.voltages[1] == doctest::Approx(8.f));
}

TEST_CASE("Gate poly: per-voice gate timing") {
	GateCore m;
	m.set_samplerate(48000.f);
	m.set_param(GateInfo::KnobLength, 0.1f); // ~100ms gate
	m.set_param(GateInfo::KnobDelay, 0.0f);	 // no delay

	auto in = m.get_poly_input_buffer(GateInfo::InputGate_In);
	auto out = m.get_poly_output_buffer(GateInfo::OutputGate_Out);

	patch(m, GateInfo::InputGate_In, in, 2);

	// Trigger voice 1 only
	in.voltages[1] = 8.f;
	for (int i = 0; i < 100; i++)
		m.update();
	in.voltages[1] = 0.f;
	m.update();

	CHECK(*out.channels == 2);
	CHECK(out.voltages[0] == doctest::Approx(0.f));
	CHECK(out.voltages[1] == doctest::Approx(8.f));

	// After the gate length passes, voice 1 goes low again
	for (int i = 0; i < 48000; i++)
		m.update();
	CHECK(out.voltages[1] == doctest::Approx(0.f));
}

TEST_CASE("LPG poly: per-voice ping opens its own gate") {
	LPGCore m;
	m.set_samplerate(48000.f);
	m.set_param(LPGInfo::KnobDecay, 0.7f);

	auto audio = m.get_poly_input_buffer(LPGInfo::AudioInInput);
	auto ping = m.get_poly_input_buffer(LPGInfo::InputPing);
	auto out = m.get_poly_output_buffer(LPGInfo::AudioOutOutput);
	m.mark_output_patched(LPGInfo::AudioOutOutput);

	patch(m, LPGInfo::AudioInInput, audio, 2);
	patch(m, LPGInfo::InputPing, ping, 2);

	// Closed gates: no output
	float peak[2]{};
	for (int i = 0; i < 4800; i++) {
		audio.voltages[0] = (i / 32) % 2 ? 5.f : -5.f;
		audio.voltages[1] = (i / 32) % 2 ? 5.f : -5.f;
		m.update();
		for (int ch = 0; ch < 2; ch++)
			peak[ch] = std::max(peak[ch], std::abs(out.voltages[ch]));
	}
	CHECK(*out.channels == 2);
	CHECK(peak[0] < 0.1f);
	CHECK(peak[1] < 0.1f);

	// Ping voice 1: only voice 1 opens
	ping.voltages[1] = 8.f;
	for (int i = 0; i < 100; i++)
		m.update();
	ping.voltages[1] = 0.f;

	peak[0] = peak[1] = 0.f;
	for (int i = 0; i < 4800; i++) {
		audio.voltages[0] = (i / 32) % 2 ? 5.f : -5.f;
		audio.voltages[1] = (i / 32) % 2 ? 5.f : -5.f;
		m.update();
		for (int ch = 0; ch < 2; ch++)
			peak[ch] = std::max(peak[ch], std::abs(out.voltages[ch]));
	}
	CHECK(peak[1] > 1.f);
	CHECK(peak[0] < peak[1] * 0.05f);
}

TEST_CASE("SH poly: sides independently poly, per-voice sampling") {
	SHCore m;
	auto val1 = m.get_poly_input_buffer(SHInfo::InputCh__1_Clock_In);
	auto trig1 = m.get_poly_input_buffer(SHInfo::InputCh__1_Sample_In);
	auto out1 = m.get_poly_output_buffer(0);
	auto out2 = m.get_poly_output_buffer(1);

	patch(m, SHInfo::InputCh__1_Clock_In, val1, 3);
	patch(m, SHInfo::InputCh__1_Sample_In, trig1, 1);
	val1.voltages[0] = 1.f;
	val1.voltages[1] = 2.f;
	val1.voltages[2] = 3.f;

	m.update();
	CHECK(*out1.channels == 3);
	CHECK(*out2.channels == 1);
	CHECK(out1.voltages[0] == doctest::Approx(0.f)); // nothing sampled yet

	// One trigger samples all three voices at once
	trig1.voltages[0] = 5.f;
	m.update();
	trig1.voltages[0] = 0.f;
	m.update();

	CHECK(out1.voltages[0] == doctest::Approx(1.f));
	CHECK(out1.voltages[1] == doctest::Approx(2.f));
	CHECK(out1.voltages[2] == doctest::Approx(3.f));

	// Held values stay after the input changes
	val1.voltages[0] = 9.f;
	m.update();
	CHECK(out1.voltages[0] == doctest::Approx(1.f));
}

TEST_CASE("Switch14 poly: all outputs follow signal input channels") {
	Switch14Core m;
	auto in = m.get_poly_input_buffer(Switch14Info::InputSignal_In);
	auto cv = m.get_poly_input_buffer(Switch14Info::InputCv);
	auto out1 = m.get_poly_output_buffer(Switch14Info::OutputCh__1_Out);
	auto out2 = m.get_poly_output_buffer(Switch14Info::OutputCh__2_Out);

	patch(m, Switch14Info::InputSignal_In, in, 2);
	in.voltages[0] = 3.f;
	in.voltages[1] = 6.f;

	// No clock: step 0 is active
	m.update();
	CHECK(*out1.channels == 2);
	CHECK(*out2.channels == 2);
	CHECK(out1.voltages[0] == doctest::Approx(3.f));
	CHECK(out1.voltages[1] == doctest::Approx(6.f));
	CHECK(out2.voltages[0] == doctest::Approx(0.f));

	// CV mode: CV=0 keeps everything on output 1
	patch(m, Switch14Info::InputCv, cv, 1);
	cv.voltages[0] = 0.f;
	m.update();
	CHECK(out1.voltages[1] == doctest::Approx(6.f));
	CHECK(out2.voltages[1] == doctest::Approx(0.f));
}

TEST_CASE("Switch41 poly: output count is the widest input") {
	Switch41Core m;
	auto in1 = m.get_poly_input_buffer(Switch41Info::InputCh__1_In);
	auto in2 = m.get_poly_input_buffer(Switch41Info::InputCh__2_In);
	auto out = m.get_poly_output_buffer(Switch41Info::OutputOut);

	patch(m, Switch41Info::InputCh__1_In, in1, 3);
	in1.voltages[0] = 1.f;
	in1.voltages[1] = 2.f;
	in1.voltages[2] = 3.f;
	patch(m, Switch41Info::InputCh__2_In, in2, 1);
	in2.voltages[0] = 9.f;

	// No clock: input 1 selected; output is 3 channels wide
	m.update();
	CHECK(*out.channels == 3);
	CHECK(out.voltages[0] == doctest::Approx(1.f));
	CHECK(out.voltages[1] == doctest::Approx(2.f));
	CHECK(out.voltages[2] == doctest::Approx(3.f));
}

TEST_CASE("Detune poly: per-voice shifting, output follows input channels") {
	DetuneCore m;
	m.set_samplerate(48000.f);
	m.set_param(DetuneInfo::KnobWow_Depth, 0.5f);
	m.set_param(DetuneInfo::KnobWow_Speed, 0.5f);

	auto in = m.get_poly_input_buffer(DetuneInfo::InputAudio_In);
	auto out = m.get_poly_output_buffer(DetuneInfo::OutputAudio_Out);

	patch(m, DetuneInfo::InputAudio_In, in, 2);

	float energy[2]{};
	for (int i = 0; i < 9600; i++) {
		in.voltages[0] = 0.f;
		in.voltages[1] = (i / 32) % 2 ? 5.f : -5.f;
		m.update();
		energy[0] += std::abs(out.voltages[0]);
		energy[1] += std::abs(out.voltages[1]);
	}
	CHECK(*out.channels == 2);
	CHECK(energy[1] > 100.f);
	CHECK(energy[0] == doctest::Approx(0.f));
}
