//Hack: include the .cc files (same approach as djembe_tests):
#include "CoreModules/4ms/core/FreeverbCore.cc"
#include "CoreModules/4ms/core/L4Core.cc"
#include "CoreModules/4ms/core/MNMXCore.cc"
#include "CoreModules/4ms/core/PICore.cc"
#include "CoreModules/4ms/core/SISMCore.cc"
#include "CoreModules/4ms/core/VCAMCore.cc"
#include "CoreModules/4ms/core/VerbCore.cc"

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

TEST_CASE("MNMX poly: per-voice min/max, widest input wins") {
	MNMXCore m;
	auto inA = m.get_poly_input_buffer(MNMXInfo::InputIn_A);
	auto inB = m.get_poly_input_buffer(MNMXInfo::InputIn_B);
	auto outMin = m.get_poly_output_buffer(MNMXInfo::OutputMin);
	auto outMax = m.get_poly_output_buffer(MNMXInfo::OutputMax);

	patch(m, MNMXInfo::InputIn_A, inA, 2);
	inA.voltages[0] = 1.f;
	inA.voltages[1] = 5.f;
	patch(m, MNMXInfo::InputIn_B, inB, 1);
	inB.voltages[0] = 3.f;

	m.update();
	CHECK(*outMin.channels == 2);
	CHECK(*outMax.channels == 2);
	CHECK(outMin.voltages[0] == doctest::Approx(1.f));
	CHECK(outMax.voltages[0] == doctest::Approx(3.f));
	CHECK(outMin.voltages[1] == doctest::Approx(3.f)); // B copy-highest
	CHECK(outMax.voltages[1] == doctest::Approx(5.f));
}

TEST_CASE("PI poly: per-voice gate and envelope") {
	PICore m;
	m.set_samplerate(48000.f);
	// Element-order params: GainSwitch likely first elems; set via legacy id lookup is
	// not needed -- defaults give LOW gain (2x), sensitivity knob raw 0 -> gain 0.
	// Use Sensitivity=1 to pass signal: find its param index by setting all knobs to 1.
	for (int i = 0; i < 8; i++)
		m.set_param(i, 1.0f);

	auto in = m.get_poly_input_buffer(PIInfo::InputAudio_In);
	auto gate = m.get_poly_output_buffer(PIInfo::OutputGate_Out);
	auto audio = m.get_poly_output_buffer(PIInfo::OutputAudio_Out);
	m.mark_output_patched(PIInfo::OutputGate_Out);
	m.mark_output_patched(PIInfo::OutputAudio_Out);

	patch(m, PIInfo::InputAudio_In, in, 2);
	in.voltages[0] = 0.f;
	in.voltages[1] = 5.f; // above gate threshold after gain

	for (int i = 0; i < 100; i++)
		m.update();

	CHECK(*gate.channels == 2);
	CHECK(*audio.channels == 2);
	CHECK(gate.voltages[1] == doctest::Approx(8.f));
	CHECK(gate.voltages[0] == doctest::Approx(0.f));
	CHECK(std::abs(audio.voltages[1]) > 0.5f);
}

TEST_CASE("SISM poly: per-channel widths, normalling, poly mix") {
	SISMCore m;
	auto in1 = m.get_poly_input_buffer(SISMInfo::InputCh__1_In);
	auto out1 = m.get_poly_output_buffer(SISMInfo::OutputCh__1_Out);
	auto out2 = m.get_poly_output_buffer(SISMInfo::OutputCh__2_Out);
	auto mix = m.get_poly_output_buffer(SISMInfo::OutputMix);
	for (int i = 0; i < SISMInfo::NumOutJacks; i++)
		m.mark_output_patched(i);

	// All knobs at 1.0: scale = +1, shift = +9.5V
	for (int i = 0; i < 8; i++)
		m.set_param(i, i % 2 ? 0.5f : 1.0f); // scale knobs (even) = 1, shift knobs (odd) = 0.5 (no shift)

	patch(m, SISMInfo::InputCh__1_In, in1, 3);
	in1.voltages[0] = 1.f;
	in1.voltages[1] = 2.f;
	in1.voltages[2] = 3.f;

	m.update();
	// Ch1 passes through; Ch2 normals from Ch1
	CHECK(*out1.channels == 3);
	CHECK(*out2.channels == 3);
	CHECK(out1.voltages[2] == doctest::Approx(3.f));
	CHECK(out2.voltages[2] == doctest::Approx(3.f));

	// Mix = ch1 + ch2(normalled) + 0 + 0 = 2x input per voice
	CHECK(*mix.channels == 3);
	CHECK(mix.voltages[1] == doctest::Approx(4.f));
}

TEST_CASE("VCAM poly: output width follows inputs, per-voice control") {
	VCAMCore m;
	auto inA = m.get_poly_input_buffer(VCAMInfo::InputIn_A);
	auto ctrlA1 = m.get_poly_input_buffer(VCAMInfo::InputA1_Jack);
	auto out1 = m.get_poly_output_buffer(VCAMInfo::OutputOut_1);
	for (int i = 0; i < 4; i++)
		m.mark_output_patched(i);

	// A1 level knob to full (element order: A1LevelKnob is param 0),
	// A1 button DOWN = unmuted (element order: A1Button is param 16)
	m.set_param(0, 1.0f);
	m.set_param(16, 1.0f);

	patch(m, VCAMInfo::InputIn_A, inA, 2);
	inA.voltages[0] = 5.f;
	inA.voltages[1] = 5.f;

	m.update();
	CHECK(*out1.channels == 2);
	// Unpatched control = 5V = full gain: output ~= input
	CHECK(out1.voltages[0] > 4.f);
	CHECK(out1.voltages[1] > 4.f);

	// Per-voice control CV: close voice 1 only
	patch(m, VCAMInfo::InputA1_Jack, ctrlA1, 2);
	ctrlA1.voltages[0] = 5.f;
	ctrlA1.voltages[1] = 0.f;
	m.update();
	CHECK(out1.voltages[0] > 4.f);
	CHECK(std::abs(out1.voltages[1]) < 0.1f);
}

TEST_CASE("L4 poly: outputs follow widest input, right normals to left") {
	L4Core m;
	m.set_samplerate(48000.f);
	// Levels full (audio-taper curve is tiny at mid), line-mode switch off
	for (int i = 0; i <= 8; i++)
		m.set_param(i, 1.0f);
	m.set_param(6, 0.0f); // OutputLevelSwitch: synth level

	auto in3L = m.get_poly_input_buffer(L4Info::InputCh__3_Left_In);
	auto outL = m.get_poly_output_buffer(L4Info::OutputMain_Left_Out);
	auto outR = m.get_poly_output_buffer(L4Info::OutputMain_Right_Out);
	m.mark_output_patched(L4Info::OutputMain_Left_Out);
	m.mark_output_patched(L4Info::OutputMain_Right_Out);

	patch(m, L4Info::InputCh__3_Left_In, in3L, 2);

	// AC signal so the DC blockers pass it
	float peakL[2]{}, peakR[2]{};
	for (int i = 0; i < 4800; i++) {
		in3L.voltages[0] = (i / 32) % 2 ? 2.f : -2.f;
		in3L.voltages[1] = (i / 32) % 2 ? 4.f : -4.f;
		m.update();
		for (int ch = 0; ch < 2; ch++) {
			peakL[ch] = std::max(peakL[ch], std::abs(outL.voltages[ch]));
			peakR[ch] = std::max(peakR[ch], std::abs(outR.voltages[ch]));
		}
	}
	CHECK(*outL.channels == 2);
	CHECK(*outR.channels == 2);
	// Both sides get the signal (right normalled from left), voice 1 louder
	CHECK(peakL[0] > 0.2f);
	CHECK(peakR[0] > 0.2f);
	CHECK(peakL[1] > peakL[0] * 1.5f);
}

TEST_CASE("Freeverb poly: per-voice reverb tails") {
	FreeverbCore m;
	m.set_samplerate(48000.f);
	// Element-order: SizeKnob=0, DampKnob=1, FeedbackKnob=2, MixKnob=3 (check via behavior)
	for (int i = 0; i < 4; i++)
		m.set_param(i, 0.5f);

	auto in = m.get_poly_input_buffer(FreeverbInfo::InputInput);
	auto out = m.get_poly_output_buffer(FreeverbInfo::OutputOut);
	m.mark_output_patched(FreeverbInfo::OutputOut);

	patch(m, FreeverbInfo::InputInput, in, 2);

	// Impulse into voice 1 only
	in.voltages[1] = 5.f;
	m.update();
	in.voltages[1] = 0.f;

	float energy[2]{};
	for (int i = 0; i < 9600; i++) {
		m.update();
		energy[0] += std::abs(out.voltages[0]);
		energy[1] += std::abs(out.voltages[1]);
	}
	CHECK(*out.channels == 2);
	CHECK(energy[1] > 1.f);				   // reverb tail rings
	CHECK(energy[0] < energy[1] * 0.01f);  // voice 0 stays silent
}

TEST_CASE("Verb poly: per-voice reverb tails") {
	VerbCore m;
	m.set_samplerate(48000.f);
	m.set_param(VerbInfo::KnobSize, 0.5f);
	m.set_param(VerbInfo::KnobTime, 0.5f);
	m.set_param(VerbInfo::KnobMix, 1.0f); // wet
	m.set_param(VerbInfo::KnobDamping, 0.2f);
	m.set_param(VerbInfo::KnobAp_Ratio, 0.5f);
	m.set_param(VerbInfo::KnobComb, 0.5f);

	auto in = m.get_poly_input_buffer(VerbInfo::InputAudio_In);
	auto out = m.get_poly_output_buffer(VerbInfo::OutputAudio_Out);

	patch(m, VerbInfo::InputAudio_In, in, 2);

	// Impulse into voice 0 only
	in.voltages[0] = 5.f;
	m.update();
	in.voltages[0] = 0.f;

	float energy[2]{};
	for (int i = 0; i < 9600; i++) {
		m.update();
		energy[0] += std::abs(out.voltages[0]);
		energy[1] += std::abs(out.voltages[1]);
	}
	CHECK(*out.channels == 2);
	CHECK(energy[0] > 1.f);
	CHECK(energy[1] < energy[0] * 0.01f);
}
