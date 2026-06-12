//Hack: include the .cc file (same approach as djembe_tests):
#include "CoreModules/4ms/core/SHEVCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

struct ShevUnderTest {
	SHEVCore m;
	CoreProcessor::PolyPortBuffer trigA, trigB, audioInA;
	CoreProcessor::PolyPortBuffer envA, envB, lin5vA, eorA, eofB, orOut, audioOutA, audioOutB;

	// Param ids in element order:
	enum {
		CycleA = 0, RiseARange, FallARange, RiseBRange, FallBRange, CycleB,
		LevelA, TrigModeA, RiseASlider, FallASlider, RiseBSlider, FallBSlider,
		TrigModeB, LevelB, OffsetA, OffsetB, ShapeAKnob, ShapeASlider,
		ShapeBSlider, ShapeBKnob, RiseACv, FallACv, RiseBCv, FallBCv
	};

	ShevUnderTest() {
		m.set_samplerate(48000.f);
		m.set_param(RiseARange, 0.5f);
		m.set_param(FallARange, 0.5f);
		m.set_param(RiseBRange, 0.5f);
		m.set_param(FallBRange, 0.5f);
		m.set_param(TrigModeA, 0.5f); // AR mode
		m.set_param(TrigModeB, 0.5f);
		m.set_param(RiseASlider, 0.0f); // fastest
		m.set_param(FallASlider, 0.0f);
		m.set_param(RiseBSlider, 0.0f);
		m.set_param(FallBSlider, 0.0f);
		m.set_param(LevelA, 1.0f);		 // level scale = +1
		m.set_param(LevelB, 1.0f);
		m.set_param(OffsetA, 0.5f);		 // offset = 0V
		m.set_param(OffsetB, 0.5f);
		m.set_param(ShapeASlider, 0.5f); // linear shape
		m.set_param(ShapeBSlider, 0.5f);

		trigA = m.get_poly_input_buffer(SHEVInfo::InputTrig_A);
		trigB = m.get_poly_input_buffer(SHEVInfo::InputTrig_B);
		audioInA = m.get_poly_input_buffer(SHEVInfo::InputAudio_A_In);

		envA = m.get_poly_output_buffer(SHEVInfo::OutputEnv_A_Out);
		envB = m.get_poly_output_buffer(SHEVInfo::OutputEnv_B_Out);
		lin5vA = m.get_poly_output_buffer(SHEVInfo::OutputLin_5V_A);
		eorA = m.get_poly_output_buffer(SHEVInfo::OutputEor_A);
		eofB = m.get_poly_output_buffer(SHEVInfo::OutputEof_B);
		orOut = m.get_poly_output_buffer(SHEVInfo::OutputOr);
		audioOutA = m.get_poly_output_buffer(SHEVInfo::OutputOut_A);
		audioOutB = m.get_poly_output_buffer(SHEVInfo::OutputOut_B);

		for (int out_id = 0; out_id < SHEVInfo::NumOutJacks; out_id++)
			m.mark_output_patched(out_id);
	}

	void patch(int input_id, CoreProcessor::PolyPortBuffer const &buf, unsigned chans) {
		m.mark_input_patched(input_id);
		*buf.channels = chans;
	}

	std::array<float, 4> run_peak(unsigned num_samples, CoreProcessor::PolyPortBuffer const &out) {
		std::array<float, 4> peak{};
		for (unsigned i = 0; i < num_samples; i++) {
			m.update();
			for (unsigned ch = 0; ch < *out.channels; ch++)
				peak[ch] = std::max(peak[ch], std::abs(out.voltages[ch]));
		}
		return peak;
	}

	void pulse(CoreProcessor::PolyPortBuffer const &in, unsigned chan) {
		in.voltages[chan] = 8.f;
		for (unsigned i = 0; i < 100; i++)
			m.update();
		in.voltages[chan] = 0.f;
	}
};

} // namespace

TEST_CASE("SHEV poly: per-side channel counts (incl Lin 5V)") {
	ShevUnderTest t;

	t.m.update();
	CHECK(*t.envA.channels == 1);
	CHECK(*t.lin5vA.channels == 1);
	CHECK(*t.audioOutA.channels == 1);

	t.patch(SHEVInfo::InputTrig_A, t.trigA, 3);
	t.patch(SHEVInfo::InputTrig_B, t.trigB, 2);
	t.patch(SHEVInfo::InputAudio_A_In, t.audioInA, 2);
	t.m.update();

	CHECK(*t.envA.channels == 3);
	CHECK(*t.lin5vA.channels == 3);
	CHECK(*t.eorA.channels == 3);
	CHECK(*t.envB.channels == 2);
	CHECK(*t.eofB.channels == 2);
	CHECK(*t.orOut.channels == 3);
	CHECK(*t.audioOutA.channels == 2);
	// Audio In B unpatched: B output follows A's input channels
	CHECK(*t.audioOutB.channels == 2);
}

TEST_CASE("SHEV poly: each trig channel fires its own envelope") {
	ShevUnderTest t;
	t.patch(SHEVInfo::InputTrig_A, t.trigA, 3);

	t.pulse(t.trigA, 2);
	auto env = t.run_peak(48000, t.envA);
	CHECK(env[2] > 3.f);
	CHECK(env[0] < 0.05f);
	CHECK(env[1] < 0.05f);

	// Lin 5V Out carries the raw (unscaled) envelope per channel
	t.pulse(t.trigA, 0);
	auto lin = t.run_peak(48000, t.lin5vA);
	CHECK(lin[0] > 2.f);
	CHECK(lin[1] < 0.05f);
}

TEST_CASE("SHEV poly: highest envelope controls all upper audio channels") {
	ShevUnderTest t;
	t.patch(SHEVInfo::InputTrig_A, t.trigA, 2);
	t.patch(SHEVInfo::InputAudio_A_In, t.audioInA, 4);
	for (unsigned ch = 0; ch < 4; ch++)
		t.audioInA.voltages[ch] = 5.f;

	// VCA closed before any trigger
	auto quiet = t.run_peak(1000, t.audioOutA);
	for (unsigned ch = 0; ch < 4; ch++)
		CHECK(quiet[ch] < 0.05f);

	// Envelope 1 controls audio channels 1, 2, 3; channel 0 stays closed
	t.pulse(t.trigA, 1);
	auto peak = t.run_peak(48000, t.audioOutA);
	CHECK(peak[1] > 1.f);
	CHECK(peak[2] > 1.f);
	CHECK(peak[3] > 1.f);
	CHECK(peak[0] < 0.05f);
}
