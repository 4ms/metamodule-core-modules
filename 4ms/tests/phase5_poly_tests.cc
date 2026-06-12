//Hack: include the .cc files (same approach as djembe_tests):
// #include "CoreModules/4ms/core/DLDCore.cc"
#include "CoreModules/4ms/core/DjembeCore_neon.hh"
#include "CoreModules/4ms/core/MPEGCore.cc"
#include "CoreModules/4ms/core/PEGCore.cc"

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

TEST_CASE("MPEG poly: per-voice envelopes from poly ping/trigger") {
	MPEGCore m;
	m.set_samplerate(48000.f);
	// Element-order knobs default 0; Scale knob mid for visible envelope
	// (ScaleKnob is element 0..3 range; set all 4 knobs mid)
	for (int i = 0; i < 4; i++)
		m.set_param(i, 0.5f);

	auto ping = m.get_poly_input_buffer(MPEGInfo::InputPing_Jack);
	auto envOut = m.get_poly_output_buffer(MPEGInfo::Output_5V_Env_Out);
	m.mark_output_patched(MPEGInfo::Output_5V_Env_Out);

	patch(m, MPEGInfo::InputPing_Jack, ping, 2);

	// Ping voice 1 twice (sets its clock and starts the envelope)
	for (int rep = 0; rep < 2; rep++) {
		ping.voltages[1] = 5.f;
		for (int i = 0; i < 100; i++)
			m.update();
		ping.voltages[1] = 0.f;
		for (int i = 0; i < 2300; i++)
			m.update();
	}

	float peak[2]{};
	for (int i = 0; i < 9600; i++) {
		m.update();
		for (int ch = 0; ch < 2; ch++)
			peak[ch] = std::max(peak[ch], envOut.voltages[ch]);
	}

	CHECK(*envOut.channels == 2);
	CHECK(peak[1] > 1.f);
	CHECK(peak[0] < 0.05f);
}

TEST_CASE("PEG poly: per-side voice counts and per-voice envelopes") {
	PEGCore m;
	m.set_samplerate(48000.f);

	auto pingRed = m.get_poly_input_buffer(PEGInfo::InputPing_Red_Jack);
	auto envRed = m.get_poly_output_buffer(PEGInfo::OutputP5V_Env_Red);
	auto envBlue = m.get_poly_output_buffer(PEGInfo::OutputP5V_Env_Blue);
	m.mark_output_patched(PEGInfo::OutputP5V_Env_Red);
	m.mark_output_patched(PEGInfo::OutputP5V_Env_Blue);

	patch(m, PEGInfo::InputPing_Red_Jack, pingRed, 3);
	m.update();
	CHECK(*envRed.channels == 3);
	CHECK(*envBlue.channels == 1);

	// Ping red voice 2 twice: its envelope runs, other voices stay at rest
	for (int rep = 0; rep < 2; rep++) {
		pingRed.voltages[2] = 5.f;
		for (int i = 0; i < 100; i++)
			m.update();
		pingRed.voltages[2] = 0.f;
		for (int i = 0; i < 2300; i++)
			m.update();
	}

	float peak[3]{};
	for (int i = 0; i < 9600; i++) {
		m.update();
		for (int ch = 0; ch < 3; ch++)
			peak[ch] = std::max(peak[ch], envRed.voltages[ch]);
	}

	CHECK(peak[2] > 1.f);
	CHECK(peak[0] < 0.05f);
	CHECK(peak[1] < 0.05f);
}

TEST_CASE("Djembe poly: each trigger channel fires its own drum") {
	DjembeCoreNeon m;
	m.set_samplerate(48000.f);
	m.set_param(0, 0.3f); // pitch knob
	m.set_param(1, 1.0f); // gain

	auto trig = m.get_poly_input_buffer(DjembeInfo::InputTrigger_In);
	auto pitch = m.get_poly_input_buffer(0); // pitch CV (jack index mapping follows original code)
	auto out = m.get_poly_output_buffer(0);

	patch(m, DjembeInfo::InputTrigger_In, trig, 3);
	patch(m, 0, pitch, 2);
	pitch.voltages[0] = 0.f;
	pitch.voltages[1] = 2.f;

	m.update();
	CHECK(*out.channels == 3);

	// Trigger voice 1 only
	trig.voltages[1] = 8.f;
	for (int i = 0; i < 100; i++)
		m.update();
	trig.voltages[1] = 0.f;

	float energy[3]{};
	for (int i = 0; i < 24000; i++) {
		m.update();
		for (int ch = 0; ch < 3; ch++)
			energy[ch] += std::abs(out.voltages[ch]);
	}
	CHECK(energy[1] > 10.f);
	CHECK(energy[0] < energy[1] * 0.01f);
	CHECK(energy[2] < energy[1] * 0.01f);
}

// TEST_CASE("DLD poly: per-side voice counts and per-voice dry path") {
// 	// DLD is ~64MB (8MB delay buffer per voice per side): heap-allocate like the player does
// 	auto m_ptr = std::make_unique<DLDCore>();
// 	auto &m = *m_ptr;
// 	m.set_samplerate(48000.f);
// 	// Mix knobs at 0 = dry; element order params: set Time/Feedback/DelayFeed/Mix
// 	// for both sides via legacy ids
// 	m.set_param(DLDInfo::KnobMix_A, 0.0f);
// 	m.set_param(DLDInfo::KnobMix_B, 0.0f);

// 	auto inA = m.get_poly_input_buffer(DLDInfo::InputIn_A);
// 	auto outA = m.get_poly_output_buffer(DLDInfo::OutputOut_A);
// 	auto outB = m.get_poly_output_buffer(DLDInfo::OutputOut_B);
// 	m.mark_output_patched(DLDInfo::OutputOut_A);
// 	m.mark_output_patched(DLDInfo::OutputOut_B);

// 	patch(m, DLDInfo::InputIn_A, inA, 2);

// 	m.update();
// 	CHECK(*outA.channels == 2);
// 	// In B unpatched: B follows A's voice count
// 	CHECK(*outB.channels == 2);

// 	// Drive voice 1 with audio; voice 0 silent (dry path passes within a block)
// 	float peak[2]{};
// 	for (int i = 0; i < 9600; i++) {
// 		inA.voltages[0] = 0.f;
// 		inA.voltages[1] = (i / 32) % 2 ? 4.f : -4.f;
// 		m.update();
// 		for (int ch = 0; ch < 2; ch++)
// 			peak[ch] = std::max(peak[ch], std::abs(outA.voltages[ch]));
// 	}

// 	CHECK(peak[1] > 1.f);
// 	CHECK(peak[0] < 0.05f);
// }
