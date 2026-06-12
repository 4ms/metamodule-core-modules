//Hack: include the .cc file (same approach as djembe_tests):
#include "CoreModules/4ms/core/FMCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

struct FmUnderTest {
	FMCore m;
	CoreProcessor::PolyPortBuffer carrier;
	CoreProcessor::PolyPortBuffer modulator;
	CoreProcessor::PolyPortBuffer shapeCv;
	CoreProcessor::PolyPortBuffer out;

	FmUnderTest() {
		m.set_samplerate(48000.f);
		m.set_param(FMInfo::KnobPitch, 0.5f); // ~113 Hz
		m.set_param(FMInfo::KnobMix, 0.0f);	  // carrier only
		m.set_param(FMInfo::KnobIndex, 0.0f); // no FM
		m.set_param(FMInfo::KnobShape, 0.0f); // sine

		carrier = m.get_poly_input_buffer(FMInfo::InputV_Oct_Carrier);
		modulator = m.get_poly_input_buffer(FMInfo::InputV_Oct_Modulator);
		shapeCv = m.get_poly_input_buffer(FMInfo::InputShape_Cv_In);
		out = m.get_poly_output_buffer(FMInfo::OutputAudio_Out);
	}

	void patch(int input_id, CoreProcessor::PolyPortBuffer const &buf, unsigned chans) {
		m.mark_input_patched(input_id);
		*buf.channels = chans;
	}

	// Run, counting zero crossings per channel (frequency proxy: crossings ~= 2*freq*duration)
	std::array<int, 4> crossings(unsigned num_samples) {
		std::array<int, 4> count{};
		std::array<float, 4> last{};
		for (unsigned i = 0; i < num_samples; i++) {
			m.update();
			for (unsigned ch = 0; ch < *out.channels; ch++) {
				float cur = out.voltages[ch];
				if (i > 0 && (cur > 0.f) != (last[ch] > 0.f))
					count[ch]++;
				last[ch] = cur;
			}
		}
		return count;
	}

	// Run, returning the mean |output| per channel (sine ~5.1V, square ~8V at full scale)
	std::array<float, 4> mean_level(unsigned num_samples) {
		std::array<float, 4> sum{};
		for (unsigned i = 0; i < num_samples; i++) {
			m.update();
			for (unsigned ch = 0; ch < *out.channels; ch++)
				sum[ch] += std::abs(out.voltages[ch]);
		}
		for (auto &s : sum)
			s /= float(num_samples);
		return sum;
	}
};

} // namespace

TEST_CASE("FM poly: output channel count follows V/Oct Carrier") {
	FmUnderTest t;

	t.m.update();
	CHECK(*t.out.channels == 1);

	t.patch(FMInfo::InputV_Oct_Carrier, t.carrier, 3);
	t.m.update();
	CHECK(*t.out.channels == 3);

	t.m.mark_input_unpatched(FMInfo::InputV_Oct_Carrier);
	t.m.update();
	CHECK(*t.out.channels == 1);
}

TEST_CASE("FM poly: each voice tracks its own carrier pitch") {
	FmUnderTest t;

	t.patch(FMInfo::InputV_Oct_Carrier, t.carrier, 2);
	t.carrier.voltages[0] = 0.f; // ~113 Hz
	t.carrier.voltages[1] = 1.f; // 1 octave up: ~226 Hz

	auto c = t.crossings(48000);
	CHECK(c[0] > 180);
	CHECK(c[0] < 280);
	float ratio = float(c[1]) / float(c[0]);
	CHECK(ratio > 1.9f);
	CHECK(ratio < 2.1f);
}

TEST_CASE("FM poly: modulator V/Oct input overrides the ratio knobs") {
	FmUnderTest t;
	t.m.set_param(FMInfo::KnobMix, 1.0f); // modulator only

	// Unpatched: modulator = basePitch * ratio (ratio = 1) -> ~113 Hz
	auto c = t.crossings(48000);
	CHECK(c[0] > 180);
	CHECK(c[0] < 280);

	// Patched at +1V: modulator = basePitch * 2 -> ~226 Hz
	t.patch(FMInfo::InputV_Oct_Modulator, t.modulator, 1);
	t.modulator.voltages[0] = 1.f;
	auto c2 = t.crossings(48000);
	CHECK(c2[0] > 400);
	CHECK(c2[0] < 510);

	// Unpatching reverts to the ratio knobs
	t.m.mark_input_unpatched(FMInfo::InputV_Oct_Modulator);
	auto c3 = t.crossings(48000);
	CHECK(c3[0] > 180);
	CHECK(c3[0] < 280);
}

TEST_CASE("FM poly: mono shape CV is copied to all voices") {
	FmUnderTest t;
	t.m.set_param(FMInfo::KnobShape_Cv, 1.0f); // full shape CV amount

	t.patch(FMInfo::InputV_Oct_Carrier, t.carrier, 3);
	t.carrier.voltages[0] = 0.f;
	t.carrier.voltages[1] = 1.f;
	t.carrier.voltages[2] = 2.f;

	// Sine: mean |out| = 8 * 2/pi ~ 5.1V on every voice
	auto sine = t.mean_level(48000);
	for (unsigned ch = 0; ch < 3; ch++) {
		CHECK(sine[ch] > 4.6f);
		CHECK(sine[ch] < 5.6f);
	}

	// Shape CV at 5V drives all voices to square: mean |out| ~ 8V
	t.patch(FMInfo::InputShape_Cv_In, t.shapeCv, 1);
	t.shapeCv.voltages[0] = 5.f;
	auto square = t.mean_level(48000);
	for (unsigned ch = 0; ch < 3; ch++)
		CHECK(square[ch] > 7.5f);
}

TEST_CASE("FM poly: mono operation via set_input still works") {
	FmUnderTest t;

	auto base = t.crossings(48000);
	CHECK(base[0] > 180);
	CHECK(base[0] < 280);

	// +1V via the mono path doubles the frequency
	t.m.mark_input_patched(FMInfo::InputV_Oct_Carrier);
	t.m.set_input(FMInfo::InputV_Oct_Carrier, 5.f); // 5V -> 32x
	auto fast = t.crossings(48000);
	CHECK(fast[0] > base[0] * 20);
	CHECK(t.m.get_output(FMInfo::OutputAudio_Out) == t.out.voltages[0]);
}

TEST_CASE("FM poly: bypass silences all channels") {
	FmUnderTest t;

	t.patch(FMInfo::InputV_Oct_Carrier, t.carrier, 4);
	t.m.update();
	CHECK(*t.out.channels == 4);

	t.m.bypassed = true;
	auto level = t.mean_level(100);
	for (unsigned ch = 0; ch < 4; ch++)
		CHECK(level[ch] == 0.f);
}
