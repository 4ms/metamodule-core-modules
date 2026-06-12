//Hack: include the .cc files (same approach as djembe_tests):
#include "CoreModules/4ms/core/Atvert2Core.cc"
#include "CoreModules/4ms/core/BPFCore.cc"
#include "CoreModules/4ms/core/HPFCore.cc"
#include "CoreModules/4ms/core/OctaveCore.cc"
#include "CoreModules/4ms/core/PanCore.cc"
#include "CoreModules/4ms/core/SlewCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

// Patch an input and set its poly channel count
void patch(CoreProcessor &m, int input_id, CoreProcessor::PolyPortBuffer const &buf, unsigned chans) {
	m.mark_input_patched(input_id);
	*buf.channels = chans;
}

} // namespace

TEST_CASE("Octave poly: per-voice offset, CV copied to upper voices") {
	OctaveCore m;
	auto in = m.get_poly_input_buffer(OctaveInfo::InputInput);
	auto cv = m.get_poly_input_buffer(OctaveInfo::InputCv);
	auto out = m.get_poly_output_buffer(OctaveInfo::OutputOut);
	m.set_param(OctaveInfo::KnobOctave, 0.5f); // no offset

	m.update();
	CHECK(*out.channels == 1);

	patch(m, OctaveInfo::InputInput, in, 2);
	in.voltages[0] = 1.f;
	in.voltages[1] = 2.f;
	patch(m, OctaveInfo::InputCv, cv, 1);
	cv.voltages[0] = 1.2f; // rounds to +1 octave, both voices

	m.update();
	CHECK(*out.channels == 2);
	CHECK(out.voltages[0] == doctest::Approx(2.f));
	CHECK(out.voltages[1] == doctest::Approx(3.f));
	CHECK(m.get_output(OctaveInfo::OutputOut) == out.voltages[0]);

	m.mark_input_unpatched(OctaveInfo::InputInput);
	m.update();
	CHECK(*out.channels == 1);
}

TEST_CASE("Slew poly: per-voice slew state") {
	SlewCore m;
	m.set_samplerate(48000.f);
	m.set_param(SlewInfo::KnobRise, 1.0f); // slow-ish rise
	m.set_param(SlewInfo::KnobFall, 1.0f);

	auto in = m.get_poly_input_buffer(SlewInfo::InputSignal_In);
	auto out = m.get_poly_output_buffer(SlewInfo::OutputSlewed_Out);

	patch(m, SlewInfo::InputSignal_In, in, 2);
	in.voltages[0] = 0.f;
	in.voltages[1] = 5.f; // step on voice 1 only

	for (int i = 0; i < 100; i++)
		m.update();

	CHECK(*out.channels == 2);
	CHECK(out.voltages[0] == doctest::Approx(0.f));
	CHECK(out.voltages[1] > 0.01f);  // rising...
	CHECK(out.voltages[1] < 4.9f);	 // ...but not instantly
}

TEST_CASE("Pan poly: both outputs follow input channels, CV copied") {
	PanCore m;
	auto in = m.get_poly_input_buffer(PanInfo::InputAudio_In);
	auto cv = m.get_poly_input_buffer(PanInfo::InputPan_Cv_In);
	auto left = m.get_poly_output_buffer(PanInfo::OutputCh__1_Out);
	auto right = m.get_poly_output_buffer(PanInfo::OutputCh__2_Out);
	m.set_param(PanInfo::KnobPan, 0.f); // full left

	patch(m, PanInfo::InputAudio_In, in, 3);
	in.voltages[0] = 2.f;
	in.voltages[1] = 4.f;
	in.voltages[2] = 6.f;

	m.update();
	CHECK(*left.channels == 3);
	CHECK(*right.channels == 3);
	for (unsigned ch = 0; ch < 3; ch++) {
		CHECK(left.voltages[ch] == doctest::Approx(in.voltages[ch]));
		CHECK(right.voltages[ch] == doctest::Approx(0.f));
	}

	// Pan CV +5V pans all voices full right
	patch(m, PanInfo::InputPan_Cv_In, cv, 1);
	cv.voltages[0] = 5.f;
	m.update();
	for (unsigned ch = 0; ch < 3; ch++) {
		CHECK(left.voltages[ch] == doctest::Approx(0.f));
		CHECK(right.voltages[ch] == doctest::Approx(in.voltages[ch]));
	}
}

TEST_CASE("Atvert2 poly: sides are independently poly, unpatched side normals to 5V") {
	Atvert2Core m;
	auto in1 = m.get_poly_input_buffer(Atvert2Info::InputCh__1_In);
	auto out1 = m.get_poly_output_buffer(Atvert2Info::OutputCh__1_Out);
	auto out2 = m.get_poly_output_buffer(Atvert2Info::OutputCh__2_Out);
	m.set_param(Atvert2Info::KnobCh__1, 1.0f); // +1
	m.set_param(Atvert2Info::KnobCh__2, 0.0f); // -1

	patch(m, Atvert2Info::InputCh__1_In, in1, 2);
	in1.voltages[0] = 2.f;
	in1.voltages[1] = -3.f;

	m.update();
	CHECK(*out1.channels == 2);
	CHECK(out1.voltages[0] == doctest::Approx(2.f));
	CHECK(out1.voltages[1] == doctest::Approx(-3.f));

	// Side 2 unpatched: mono, 5V * level
	CHECK(*out2.channels == 1);
	CHECK(out2.voltages[0] == doctest::Approx(-5.f));
}

TEST_CASE("BPF poly: per-voice filtering (DC blocked, audio passes)") {
	BPFCore m;
	m.set_samplerate(48000.f);
	m.set_param(BPFInfo::KnobCutoff, 0.5f);
	m.set_param(BPFInfo::KnobQ, 0.f);

	auto in = m.get_poly_input_buffer(BPFInfo::InputAudio_In);
	auto out = m.get_poly_output_buffer(BPFInfo::OutputBandpass_Out);
	patch(m, BPFInfo::InputAudio_In, in, 2);

	// Voice 0: DC (blocked by a bandpass). Voice 1: ~750Hz square (passes).
	float energy[2]{};
	for (int i = 0; i < 24000; i++) {
		in.voltages[0] = 5.f;
		in.voltages[1] = (i / 32) % 2 ? 5.f : -5.f;
		m.update();
		if (i > 4800) { // skip DC settle
			energy[0] += std::abs(out.voltages[0]);
			energy[1] += std::abs(out.voltages[1]);
		}
	}
	CHECK(*out.channels == 2);
	CHECK(energy[1] > energy[0] * 10.f);
}

TEST_CASE("HPF poly: per-voice filtering (DC blocked, audio passes)") {
	HPFCore m;
	m.set_samplerate(48000.f);
	m.set_param(HPFInfo::KnobCutoff, 0.5f);

	auto in = m.get_poly_input_buffer(HPFInfo::InputAudio_In);
	auto out = m.get_poly_output_buffer(HPFInfo::OutputAudio_Out);
	patch(m, HPFInfo::InputAudio_In, in, 2);

	float energy[2]{};
	for (int i = 0; i < 24000; i++) {
		in.voltages[0] = 5.f;
		in.voltages[1] = (i / 8) % 2 ? 5.f : -5.f; // ~3kHz square
		m.update();
		if (i > 4800) {
			energy[0] += std::abs(out.voltages[0]);
			energy[1] += std::abs(out.voltages[1]);
		}
	}
	CHECK(*out.channels == 2);
	CHECK(energy[1] > energy[0] * 10.f);
}
