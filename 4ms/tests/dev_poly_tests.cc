//Hack: include the .cc file (same approach as djembe_tests):
#include "CoreModules/4ms/core/DEVCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

struct DevUnderTest {
	DEVCore m;
	CoreProcessor::PolyPortBuffer trigA, trigB, audioInA, audioInB, vcaCvA;
	CoreProcessor::PolyPortBuffer envA, envB, eorA, eofB, orOut, audioOutA, audioOutB;

	// Param ids in element order:
	enum {
		RiseARange = 0, FallARange, CycleA, CycleB, RiseBRange, FallBRange,
		RiseASlider, FallASlider, LevelA, LevelB, OffsetA, OffsetB,
		RiseBSlider, FallBSlider, RiseACv, FallACv, RiseBCv, FallBCv
	};

	DevUnderTest() {
		m.set_samplerate(48000.f);
		m.set_param(RiseARange, 0.5f);
		m.set_param(FallARange, 0.5f);
		m.set_param(RiseBRange, 0.5f);
		m.set_param(FallBRange, 0.5f);
		m.set_param(RiseASlider, 0.0f); // fastest
		m.set_param(FallASlider, 0.0f);
		m.set_param(RiseBSlider, 0.0f);
		m.set_param(FallBSlider, 0.0f);
		m.set_param(LevelA, 1.0f);	// level scale = +1
		m.set_param(LevelB, 1.0f);
		m.set_param(OffsetA, 0.5f); // offset = 0V
		m.set_param(OffsetB, 0.5f);

		trigA = m.get_poly_input_buffer(DEVInfo::InputTrig_A);
		trigB = m.get_poly_input_buffer(DEVInfo::InputTrig_B);
		audioInA = m.get_poly_input_buffer(DEVInfo::InputAudio_A_In);
		audioInB = m.get_poly_input_buffer(DEVInfo::InputAudio_B_In);
		vcaCvA = m.get_poly_input_buffer(DEVInfo::InputVca_Cv_A);

		envA = m.get_poly_output_buffer(DEVInfo::OutputEnv_A_Out);
		envB = m.get_poly_output_buffer(DEVInfo::OutputEnv_B_Out);
		eorA = m.get_poly_output_buffer(DEVInfo::OutputEor_A);
		eofB = m.get_poly_output_buffer(DEVInfo::OutputEof_B);
		orOut = m.get_poly_output_buffer(DEVInfo::OutputOr);
		audioOutA = m.get_poly_output_buffer(DEVInfo::OutputAudio_A_Out);
		audioOutB = m.get_poly_output_buffer(DEVInfo::OutputAudio_B_Out);

		for (int out_id = 0; out_id < DEVInfo::NumOutJacks; out_id++)
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

TEST_CASE("DEV poly: per-side channel counts") {
	DevUnderTest t;

	t.m.update();
	CHECK(*t.envA.channels == 1);
	CHECK(*t.envB.channels == 1);
	CHECK(*t.audioOutA.channels == 1);
	CHECK(*t.audioOutB.channels == 1);
	CHECK(*t.orOut.channels == 1);

	t.patch(DEVInfo::InputTrig_A, t.trigA, 2);
	t.patch(DEVInfo::InputTrig_B, t.trigB, 3);
	t.patch(DEVInfo::InputAudio_A_In, t.audioInA, 2);
	t.m.update();

	CHECK(*t.envA.channels == 2);
	CHECK(*t.eorA.channels == 2);
	CHECK(*t.envB.channels == 3);
	CHECK(*t.eofB.channels == 3);
	CHECK(*t.orOut.channels == 3);
	CHECK(*t.audioOutA.channels == 2);
	// Audio In B is unpatched: B output follows A's input channels
	CHECK(*t.audioOutB.channels == 2);

	t.patch(DEVInfo::InputAudio_B_In, t.audioInB, 4);
	t.m.update();
	CHECK(*t.audioOutB.channels == 4);
}

TEST_CASE("DEV poly: each trig channel fires its own envelope, with EOR") {
	DevUnderTest t;
	t.patch(DEVInfo::InputTrig_A, t.trigA, 2);

	t.pulse(t.trigA, 1);
	auto env = t.run_peak(48000, t.envA);
	CHECK(env[1] > 3.f);
	CHECK(env[0] < 0.05f);

	t.pulse(t.trigA, 0);
	auto eor = t.run_peak(48000, t.eorA);
	CHECK(eor[0] == doctest::Approx(8.f));
}

TEST_CASE("DEV poly: audio B normalled from poly audio A") {
	DevUnderTest t;
	t.patch(DEVInfo::InputTrig_B, t.trigB, 1);
	t.patch(DEVInfo::InputAudio_A_In, t.audioInA, 2);
	t.audioInA.voltages[0] = 5.f;
	t.audioInA.voltages[1] = 5.f;

	// Side B's envelope opens side B's VCA on both (normalled) channels
	t.pulse(t.trigB, 0);
	auto peakB = t.run_peak(48000, t.audioOutB);
	CHECK(*t.audioOutB.channels == 2);
	CHECK(peakB[0] > 1.f);
	CHECK(peakB[1] > 1.f);

	// Side A was not triggered: its VCA stays closed
	auto peakA = t.run_peak(1000, t.audioOutA);
	CHECK(peakA[0] < 0.05f);
	CHECK(peakA[1] < 0.05f);
}

TEST_CASE("DEV poly: Or Out is the per-channel max of both sides") {
	DevUnderTest t;
	t.patch(DEVInfo::InputTrig_A, t.trigA, 2);
	t.patch(DEVInfo::InputTrig_B, t.trigB, 1);

	t.pulse(t.trigA, 1);
	auto peak = t.run_peak(48000, t.orOut);
	CHECK(*t.orOut.channels == 2);
	CHECK(peak[1] > 3.f);
	CHECK(peak[0] < 0.05f);

	t.pulse(t.trigB, 0);
	auto peakB = t.run_peak(48000, t.orOut);
	CHECK(peakB[0] > 3.f);
}

TEST_CASE("DEV poly: VCA CV overrides the envelope") {
	DevUnderTest t;
	t.patch(DEVInfo::InputAudio_A_In, t.audioInA, 2);
	t.audioInA.voltages[0] = 5.f;
	t.audioInA.voltages[1] = 5.f;
	t.patch(DEVInfo::InputVca_Cv_A, t.vcaCvA, 1);
	t.vcaCvA.voltages[0] = 5.f; // VCA fully open, no envelope needed

	auto peak = t.run_peak(1000, t.audioOutA);
	CHECK(peak[0] > 4.f);
	CHECK(peak[1] > 4.f);
}
