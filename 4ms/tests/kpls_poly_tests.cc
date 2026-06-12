//Hack: include the .cc file (same approach as djembe_tests):
#include "CoreModules/4ms/core/KPLSCore.cc"

#include "doctest.h"
#include <cmath>

using namespace MetaModule;

namespace
{

struct KplsUnderTest {
	KPLSCore kpls;
	CoreProcessor::PolyPortBuffer voct;
	CoreProcessor::PolyPortBuffer trig;
	CoreProcessor::PolyPortBuffer out;

	KplsUnderTest() {
		kpls.set_samplerate(48000.f);
		kpls.set_param(KPLSInfo::KnobPitch, 0.5f);
		// Decay = 0: short noise burst (no sustain/release tail) and fast string decay,
		// so voices that were not triggered measure as silent
		kpls.set_param(KPLSInfo::KnobDecay, 0.0f);
		kpls.set_param(KPLSInfo::KnobSpread, 0.5f);

		voct = kpls.get_poly_input_buffer(KPLSInfo::InputV_Oct_In);
		trig = kpls.get_poly_input_buffer(KPLSInfo::InputTrigger_In);
		out = kpls.get_poly_output_buffer(KPLSInfo::OutputAudio_Out);
	}

	// The Envelope class free-runs one attack/decay/release cycle from
	// construction, so every voice plucks once at startup (the original mono
	// KPLS does this too). Run until that has died away.
	void warmup() {
		run(96000);
	}

	// Run for num_samples, returning the accumulated |output| per channel
	std::array<float, 4> run(unsigned num_samples) {
		std::array<float, 4> energy{};
		for (unsigned i = 0; i < num_samples; i++) {
			kpls.update();
			for (unsigned ch = 0; ch < *out.channels; ch++)
				energy[ch] += std::abs(out.voltages[ch]);
		}
		return energy;
	}

	std::array<float, 4> pluck(unsigned trig_chan) {
		trig.voltages[trig_chan] = 8.f;
		run(100);
		trig.voltages[trig_chan] = 0.f;
		return run(4000);
	}
};

} // namespace

TEST_CASE("KPLS poly: returns valid poly buffers") {
	KplsUnderTest t;

	CHECK(t.voct.voltages != nullptr);
	CHECK(t.voct.channels != nullptr);
	CHECK(t.trig.voltages != nullptr);
	CHECK(t.trig.channels != nullptr);
	CHECK(t.out.voltages != nullptr);
	CHECK(t.out.channels != nullptr);
}

TEST_CASE("KPLS poly: output channel count follows V/Oct channel count") {
	KplsUnderTest t;

	// Unpatched: mono
	t.kpls.update();
	CHECK(*t.out.channels == 1);

	// Poly V/Oct: output count matches
	t.kpls.mark_input_patched(KPLSInfo::InputV_Oct_In);
	*t.voct.channels = 3;
	t.kpls.update();
	CHECK(*t.out.channels == 3);

	*t.voct.channels = 4;
	t.kpls.update();
	CHECK(*t.out.channels == 4);

	// Trig channel count does not affect output count
	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	*t.trig.channels = 2;
	*t.voct.channels = 1;
	t.kpls.update();
	CHECK(*t.out.channels == 1);

	// Unpatching V/Oct returns to mono
	t.kpls.mark_input_unpatched(KPLSInfo::InputV_Oct_In);
	t.kpls.update();
	CHECK(*t.out.channels == 1);
}

TEST_CASE("KPLS poly: each trig channel triggers its own voice") {
	KplsUnderTest t;

	t.kpls.mark_input_patched(KPLSInfo::InputV_Oct_In);
	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	*t.voct.channels = 4;
	*t.trig.channels = 4;
	t.voct.voltages[0] = 0.f;
	t.voct.voltages[1] = 1.f;
	t.voct.voltages[2] = 2.f;
	t.voct.voltages[3] = 3.f;
	t.warmup();

	// Trigger only channel 2
	auto energy = t.pluck(2);

	CHECK(energy[2] > 0.5f);
	CHECK(energy[0] < energy[2] * 0.01f);
	CHECK(energy[1] < energy[2] * 0.01f);
	CHECK(energy[3] < energy[2] * 0.01f);
}

TEST_CASE("KPLS poly: highest trig channel triggers all upper voices") {
	KplsUnderTest t;

	t.kpls.mark_input_patched(KPLSInfo::InputV_Oct_In);
	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	*t.voct.channels = 4;
	*t.trig.channels = 2;
	t.warmup();

	// Trig channel 1 (the highest of 2) drives voices 1, 2, and 3
	auto energy = t.pluck(1);

	CHECK(energy[1] > 0.5f);
	CHECK(energy[2] > 0.5f);
	CHECK(energy[3] > 0.5f);
	CHECK(energy[0] < energy[1] * 0.01f);

	// Trig channel 0 drives only voice 0
	auto energy0 = t.pluck(0);
	CHECK(energy0[0] > 0.5f);
}

TEST_CASE("Karplus: per-voice frequency sets per-voice delay") {
	Karplus k;
	k.set_samplerate(48000.f);
	k.set_spread(0.5f); // spread multiplier = 1.0055
	k.set_decay(0.f);

	const float freqs[4] = {210.f, 840.f, 105.f, 420.f};
	for (unsigned v = 0; v < 4; v++)
		k.set_frequency(v, freqs[v]);

	// Send a 1-sample impulse to all voices, then find each voice's first echo
	alignas(16) float in[4] = {1.f, 1.f, 1.f, 1.f};
	alignas(16) float out[4];
	k.update(in, out, 4);
	in[0] = in[1] = in[2] = in[3] = 0.f;

	int first_echo[4] = {-1, -1, -1, -1};
	for (int n = 1; n < 2400; n++) {
		k.update(in, out, 4);
		for (unsigned v = 0; v < 4; v++) {
			if (first_echo[v] < 0 && n > 5 && std::abs(out[v]) > 0.02f)
				first_echo[v] = n;
		}
	}

	// The first echo arrives at the voice's shortest tap delay: period/spread^5
	for (unsigned v = 0; v < 4; v++) {
		const float period = 48000.f / freqs[v];
		const float min_delay = period / std::pow(1.0055f, 5.f);
		CHECK(first_echo[v] >= int(min_delay) - 3);
		CHECK(first_echo[v] <= int(period) + 3);
	}
}

TEST_CASE("KPLS poly: voices track their own V/Oct channel") {
	KplsUnderTest t;

	t.kpls.mark_input_patched(KPLSInfo::InputV_Oct_In);
	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	*t.voct.channels = 2;
	*t.trig.channels = 2;
	t.voct.voltages[0] = 0.f; // 210Hz (period ~229 samples)
	t.voct.voltages[1] = 2.f; // 2 octaves up: 840Hz (period ~57 samples)
	t.warmup();

	t.trig.voltages[0] = 8.f;
	t.trig.voltages[1] = 8.f;
	t.run(100);
	t.trig.voltages[0] = 0.f;
	t.trig.voltages[1] = 0.f;

	// Let the noise bursts finish, then record the ringing strings
	t.run(3000);
	std::array<std::array<float, 4000>, 2> ring{};
	for (unsigned i = 0; i < 4000; i++) {
		t.kpls.update();
		ring[0][i] = t.out.voltages[0];
		ring[1][i] = t.out.voltages[1];
	}

	// The autocorrelation peak lag tracks each voice's period
	auto best_lag = [](std::array<float, 4000> const &x, unsigned min_lag, unsigned max_lag) {
		unsigned best = min_lag;
		float best_r = -1e30f;
		for (unsigned lag = min_lag; lag <= max_lag; lag++) {
			float r = 0;
			for (unsigned n = lag; n < x.size(); n++)
				r += x[n] * x[n - lag];
			if (r > best_r) {
				best_r = r;
				best = lag;
			}
		}
		return best;
	};

	auto lag0 = best_lag(ring[0], 20, 300);
	auto lag1 = best_lag(ring[1], 20, 300);
	CHECK(lag0 >= 200);
	CHECK(lag0 <= 240);
	CHECK(lag1 >= 50);
	CHECK(lag1 <= 62);
}

TEST_CASE("KPLS poly: mono operation via set_input still works") {
	KplsUnderTest t;

	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	CHECK(*t.trig.channels == 1);
	t.warmup();

	t.kpls.set_input(KPLSInfo::InputTrigger_In, 8.f);
	t.run(100);
	t.kpls.set_input(KPLSInfo::InputTrigger_In, 0.f);
	auto energy = t.run(4000);

	CHECK(*t.out.channels == 1);
	CHECK(energy[0] > 0.5f);
	CHECK(t.kpls.get_output(KPLSInfo::OutputAudio_Out) == t.out.voltages[0]);
}

TEST_CASE("KPLS poly: bypass silences all channels") {
	KplsUnderTest t;

	t.kpls.mark_input_patched(KPLSInfo::InputV_Oct_In);
	t.kpls.mark_input_patched(KPLSInfo::InputTrigger_In);
	*t.voct.channels = 4;
	*t.trig.channels = 4;
	for (unsigned ch = 0; ch < 4; ch++)
		t.trig.voltages[ch] = 8.f;
	t.run(200);

	t.kpls.bypassed = true;
	auto energy = t.run(100);
	CHECK(*t.out.channels == 4);
	for (unsigned ch = 0; ch < 4; ch++)
		CHECK(energy[ch] == 0.f);
}
