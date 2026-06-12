// EnOsc polyphony tests.
//
// The poly channel count follows the higher of the Pitch and Root CV jacks'
// channel counts. The numOsc pairs are split roughly equally among channels,
// each channel's base pair tracks that channel's pitch CV, and the Balance
// amplitude roll-off restarts at each channel's base pair.
//
// (EnOscCore.cc is also included by enosc_golden_tests.cc; the enosc data
// tables are compiled there, so this TU must not include the .cc data files.)
#include "CoreModules/4ms/core/EnOscCore.cc"

#include "doctest.h"

#include <cmath>
#include <cstring>
#include <vector>

namespace
{

using MetaModule::EnOscCore;
using Info = MetaModule::EnOscInfo;
using CH = MetaModule::CoreHelper<Info>;
using Elem = Info::Elem;

constexpr float kSampleRate = 48000.f;
constexpr unsigned kWarmupSamples = 96000; // see enosc_golden_tests.cc

struct EnOscUnderTest {
	// zeroed storage + fixed seed: byte-deterministic runs (same approach as
	// the golden tests; the adaptor models have uninitialized members)
	std::vector<std::byte> storage;
	EnOscCore *core;

	CoreProcessor::PolyPortBuffer pitch, root, outA, outB;

	EnOscUnderTest() {
		storage.assign(sizeof(EnOscCore), std::byte{});
		easiglib::Random::seed(0xE105C0DE);
		core = new (storage.data()) EnOscCore();

		core->set_samplerate(kSampleRate);
		core->mark_all_inputs_unpatched();

		// neutral settings: no twist/warp/cross-FM, flat balance, no
		// detune/spread so each pair sits on the channel's base pitch
		core->set_param(CH::param_idx<Elem::TwistSwitch>, 0.5f);
		core->set_param(CH::param_idx<Elem::WarpSwitch>, 0.5f);
		core->set_param(CH::param_idx<Elem::CrossFmSwitch>, 0.5f);
		core->set_param(CH::param_idx<Elem::ScaleSwitch>, 0.5f);
		core->set_param(CH::param_idx<Elem::ScaleKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::SpreadKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::PitchKnob>, 0.5f);
		core->set_param(CH::param_idx<Elem::BalanceKnob>, 0.5f);
		core->set_param(CH::param_idx<Elem::RootKnob>, 0.3f);
		core->set_param(CH::param_idx<Elem::CrossFmKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::TwistKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::DetuneKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::WarpKnob>, 0.f);
		core->set_param(CH::param_idx<Elem::LearnButton>, 0.f);
		core->set_param(CH::param_idx<Elem::FreezeButton>, 0.f);
		core->set_param(CH::param_idx<Elem::FreezesplitAltParam>, 0.5f);
		core->set_param(CH::param_idx<Elem::StereosplitAltParam>, 0.f);
		core->set_param(CH::param_idx<Elem::CrossfadeAltParam>, 0.4f);
		set_num_osc(16);
		core->set_param(CH::param_idx<Elem::FinetuneAltParam>, 0.5f);

		pitch = core->get_poly_input_buffer(Info::InputPitch_1V_Oct);
		root = core->get_poly_input_buffer(Info::InputRoot_1V_Oct);
		outA = core->get_poly_output_buffer(Info::OutputOut_A);
		outB = core->get_poly_output_buffer(Info::OutputOut_B);
	}

	~EnOscUnderTest() {
		core->~EnOscCore();
	}

	void set_num_osc(int n) {
		core->set_param(CH::param_idx<Elem::NumoscAltParam>, float(n - 1) / 31.f);
	}

	void run(unsigned samples) {
		for (unsigned i = 0; i < samples; i++)
			core->update();
	}

	// rising zero crossings per second on an output channel
	float measure_freq(unsigned chan, unsigned samples = 48000) {
		unsigned crossings = 0;
		float prev = 0.f;
		for (unsigned i = 0; i < samples; i++) {
			core->update();
			float v = outA.voltages[chan] + outB.voltages[chan]; // both stereo halves
			if (prev <= 0.f && v > 0.f)
				crossings++;
			prev = v;
		}
		return float(crossings) * kSampleRate / float(samples);
	}

	float measure_rms(unsigned chan, unsigned samples = 24000) {
		double sum = 0.;
		for (unsigned i = 0; i < samples; i++) {
			core->update();
			float v = outA.voltages[chan] + outB.voltages[chan];
			sum += double(v) * double(v);
		}
		return float(std::sqrt(sum / double(samples)));
	}
};

} // namespace

TEST_CASE("EnOsc poly: returns valid poly buffers") {
	EnOscUnderTest t;
	CHECK(t.pitch.voltages != nullptr);
	CHECK(t.pitch.channels != nullptr);
	CHECK(t.root.voltages != nullptr);
	CHECK(t.outA.voltages != nullptr);
	CHECK(t.outA.channels != nullptr);
	CHECK(t.outB.channels != nullptr);
}

TEST_CASE("EnOsc poly: output channels follow max of pitch/root jack channels") {
	EnOscUnderTest t;

	// nothing patched: mono
	t.run(64);
	CHECK(*t.outA.channels == 1);
	CHECK(*t.outB.channels == 1);

	// pitch jack with 3 channels
	t.core->mark_input_patched(Info::InputPitch_1V_Oct);
	*t.pitch.channels = 3;
	t.run(64);
	CHECK(*t.outA.channels == 3);
	CHECK(*t.outB.channels == 3);

	// root jack with 4 channels wins
	t.core->mark_input_patched(Info::InputRoot_1V_Oct);
	*t.root.channels = 4;
	t.run(64);
	CHECK(*t.outA.channels == 4);
	CHECK(*t.outB.channels == 4);

	// unpatching root falls back to pitch's 3
	t.core->mark_input_unpatched(Info::InputRoot_1V_Oct);
	t.run(64);
	CHECK(*t.outA.channels == 3);

	t.core->mark_input_unpatched(Info::InputPitch_1V_Oct);
	t.run(64);
	CHECK(*t.outA.channels == 1);
}

TEST_CASE("EnOsc poly: each channel's base voice tracks its pitch CV (1V/oct)") {
	EnOscUnderTest t;
	t.set_num_osc(4); // one pair per channel

	t.core->mark_input_patched(Info::InputPitch_1V_Oct);
	*t.pitch.channels = 4;
	t.pitch.voltages[0] = 0.f;
	t.pitch.voltages[1] = 1.f; // +1 octave
	t.pitch.voltages[2] = 2.f; // +2 octaves
	t.pitch.voltages[3] = 0.f;

	t.run(kWarmupSamples);

	float f0 = t.measure_freq(0);
	float f1 = t.measure_freq(1);
	float f2 = t.measure_freq(2);
	float f3 = t.measure_freq(3);

	REQUIRE(f0 > 20.f); // audible at all

	CHECK(f1 / f0 == doctest::Approx(2.f).epsilon(0.15));
	CHECK(f2 / f0 == doctest::Approx(4.f).epsilon(0.15));
	CHECK(f3 / f0 == doctest::Approx(1.f).epsilon(0.1));
}

TEST_CASE("EnOsc poly: amplitude roll-off restarts at each channel's base voice") {
	EnOscUnderTest t;
	// strong roll-off: with 4 pairs per channel, if the roll-off did NOT
	// reset per channel, the last channel would be ~balance^12 quieter
	t.core->set_param(CH::param_idx<Elem::BalanceKnob>, 0.25f);
	t.set_num_osc(16);

	t.core->mark_input_patched(Info::InputPitch_1V_Oct);
	*t.pitch.channels = 4;
	for (int c = 0; c < 4; c++)
		t.pitch.voltages[c] = 0.f; // identical pitches

	t.run(kWarmupSamples);

	float r0 = t.measure_rms(0);
	float r3 = t.measure_rms(3);

	REQUIRE(r0 > 0.01f);
	// equal pitches + per-channel reset => roughly equal loudness
	CHECK(r3 / r0 == doctest::Approx(1.f).epsilon(0.5));
}

TEST_CASE("EnOsc poly: mono behavior when nothing poly is patched") {
	EnOscUnderTest t;
	t.run(kWarmupSamples);

	// audible on channel 0, mono declared
	float r = t.measure_rms(0);
	CHECK(r > 0.01f);
	CHECK(*t.outA.channels == 1);

	// get_output (mono cable path) matches poly channel 0
	t.core->update();
	CHECK(t.core->get_output(Info::OutputOut_A) == doctest::Approx(t.outA.voltages[0]));
}
