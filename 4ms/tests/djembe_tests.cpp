//Hack: include the .cc file:
#include "CoreModules/4ms/core/DjembeCore.hh"

#include "CoreModules/4ms/core/DjembeCore_neon.hh"
#include "CoreModules/4ms/info/Djembe_info.hh"
#include "doctest.h"
#include <array>
#include <cmath>
#include <iostream>
#include <stdint.h>

TEST_CASE("Soft-neon matches non-neon output") {
	constexpr int BufferSize = 255;
	float no_outsig[256];
	float ne_outsig[256];

	{
		using namespace MetaModule;

		DjembeCore no;
		int n = 0;
		no.update();

		// no.set_input(DjembeInfo::InputPitch_Cv, 0.f);
		// no.set_input(DjembeInfo::InputHit_Cv, 0.f);
		// no.set_input(DjembeInfo::InputSharp_Cv, 0.0f);
		// no.set_input(DjembeInfo::InputStrike_Cv, 0.0f);

		// no.set_param(DjembeInfo::KnobPitch, 60.f);
		// no.set_param(DjembeInfo::KnobHit, 1.f);
		// no.set_param(DjembeInfo::KnobSharpness, 0.5f);
		// no.set_param(DjembeInfo::KnobStrike_Amt, 0.3f);

		no.set_input(0, 0.f);
		no.set_input(2, 0.f);
		no.set_input(1, 0.0f);
		no.set_input(3, 0.0f);

		no.set_param(0, 60.f);
		no.set_param(2, 1.f);
		no.set_param(1, 0.5f);
		no.set_param(3, 0.3f);

		// Trigger low for 3
		for (int i = 0; i < 3; i++) {
			no.set_input(4, 0.f);
			no.update();
			no_outsig[n++] = no.get_output(0);
		}

		// Trigger high for 1
		for (int i = 0; i < 1; i++) {
			no.set_input(4, 1.f);
			no.update();
			no_outsig[n++] = no.get_output(0);
		}

		// Trigger low for the rest of the buffer
		for (int i = 0; i < (BufferSize - 1 - 3); i++) {
			no.set_input(4, 0.f);
			no.update();
			no_outsig[n++] = no.get_output(0);
		}
	}

	{
		MetaModule::DjembeCoreNeon ne;
		int n = 0;
		ne.update();

		ne.set_input(0, 0.f);
		ne.set_input(2, 0.f);
		ne.set_input(1, 0.0f);
		ne.set_input(3, 0.0f);

		ne.set_param(0, 60.f);
		ne.set_param(2, 1.f);
		ne.set_param(1, 0.5f);
		ne.set_param(3, 0.3f);

		// ne.set_input(DjembeInfo::InputPitch_Cv, 0.f);
		// ne.set_input(DjembeInfo::InputHit_Cv, 0.f);
		// ne.set_input(DjembeInfo::InputSharp_Cv, 0.0f);
		// ne.set_input(DjembeInfo::InputStrike_Cv, 0.0f);

		// ne.set_param(DjembeInfo::KnobPitch, 60.f);
		// ne.set_param(DjembeInfo::KnobHit, 1.f);
		// ne.set_param(DjembeInfo::KnobSharpness, 0.5f);
		// ne.set_param(DjembeInfo::KnobStrike_Amt, 0.3f);

		// Trigger low for 3
		for (int i = 0; i < 3; i++) {
			ne.set_input(4, 0.f);
			ne.update();
			ne_outsig[n++] = ne.get_output(0);
		}

		// Trigger high for 1
		for (int i = 0; i < 1; i++) {
			ne.set_input(4, 1.f);
			ne.update();
			ne_outsig[n++] = ne.get_output(0);
		}

		// Trigger low for the rest of the buffer
		for (int i = 0; i < (BufferSize - 1 - 3); i++) {
			ne.set_input(4, 0.f);
			ne.update();
			ne_outsig[n++] = ne.get_output(0);
		}
	}

	for (int i = 0; i < BufferSize; i++) {
		CHECK(no_outsig[i] == doctest::Approx(ne_outsig[i]));
	}
}

// Regression for the pitch-drift bug: the slowFreq glide must advance every
// sample until it reaches the target, not only when the Pitch CV changes.
// If it only steps on Pitch-CV edges, a clock patched into Pitch CV slowly
// drifts the pitch toward the average of its two levels (audible even though
// the CV voltage itself is unchanged). Here two voices run for an identical
// number of samples (so their noise generators stay in lockstep): one is fed a
// clock on Pitch CV then held at a level, the other is held at that level the
// whole time. After both settle they are struck identically, so the entire
// ring-down must match -- it only does if the earlier clock left no lasting
// pitch offset.
TEST_CASE("Djembe: clock on Pitch CV does not drift the pitch") {
	using namespace MetaModule;

	constexpr int ClockSamples = 6000; // long enough for the buggy drift to build
	constexpr int HalfPeriod = 50;	   // ~480 Hz clock at 48k -> many edges
	constexpr int Settle = 4000;	   // > glide convergence time
	constexpr int Capture = 400;
	constexpr float HoldVolts = 1.0f;  // level both voices end up holding
	constexpr float ClockHiVolts = 3.5f;

	auto run = [](bool withClock) {
		DjembeCoreNeon dj;
		dj.mark_input_patched(0); // Pitch CV must be patched for or_last() to read it
		dj.set_param(0, 0.2f);	  // pitch knob
		dj.set_param(1, 0.5f);	  // gain
		dj.set_param(2, 1.0f);	  // sharpness
		dj.set_param(3, 0.3f);	  // strike

		auto step = [&](float pitchV, float trigV) {
			dj.set_input(0, pitchV);
			dj.set_input(4, trigV);
			dj.update();
		};

		// Phase 1: clock on Pitch CV (or steady hold) -- same number of updates either way
		for (int i = 0; i < ClockSamples; i++) {
			float v = (withClock && ((i / HalfPeriod) % 2)) ? ClockHiVolts : HoldVolts;
			step(v, 0.f);
		}
		// Phase 2: hold at the level so the glide converges (fixed) / freezes (buggy)
		for (int i = 0; i < Settle; i++)
			step(HoldVolts, 0.f);

		// Phase 3: strike once and capture the ring-down
		std::array<float, Capture> out{};
		step(HoldVolts, 1.f); // rising edge on Trigger
		for (int i = 0; i < Capture; i++) {
			step(HoldVolts, 0.f);
			out[i] = dj.get_output(0);
		}
		return out;
	};

	auto ref = run(false);
	auto tst = run(true);

	for (int i = 0; i < Capture; i++)
		CHECK(tst[i] == doctest::Approx(ref[i]));
}

// Voice-activity gating: once a struck voice rings out it is retired -- the IIR
// banks are skipped and the output is gated to exactly 0 -- and the next strike
// must re-fire it. One mono voice: strike, let it fully ring out (output reaches
// exactly 0), then strike again and confirm it sounds again.
TEST_CASE("Djembe: a retired voice re-fires on the next strike") {
	using namespace MetaModule;

	DjembeCoreNeon dj;
	dj.set_param(0, 0.2f); // pitch
	dj.set_param(1, 0.5f); // gain
	dj.set_param(2, 1.0f); // sharpness
	dj.set_param(3, 0.3f); // strike
	auto out = dj.get_poly_output_buffer(DjembeInfo::OutputAudio_Out);

	auto strike_energy = [&]() {
		dj.set_input(4, 1.f); // rising edge on Trigger
		dj.update();
		float e = std::abs(out.voltages[0]);
		for (int i = 0; i < 3000; i++) {
			dj.set_input(4, 0.f);
			dj.update();
			e += std::abs(out.voltages[0]);
		}
		return e;
	};

	const float e1 = strike_energy();
	CHECK(e1 > 0.01f); // first strike makes sound

	// Long silence: the voice rings out and retires. The gated path writes a
	// literal 0, so once retired the output is *exactly* zero (a non-gated
	// implementation would still have a tiny non-zero ring-out tail here).
	for (int i = 0; i < 96000; i++) {
		dj.set_input(4, 0.f);
		dj.update();
	}
	CHECK(out.voltages[0] == 0.f);

	const float e2 = strike_energy();
	CHECK(e2 > 0.01f); // re-fires after being retired
}
