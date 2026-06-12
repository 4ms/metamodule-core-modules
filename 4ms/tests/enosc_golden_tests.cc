// EnOsc golden-master regression tests.
//
// Captures the module's stereo output for a matrix of parameter scenarios and
// compares it sample-for-sample against committed golden files, so that
// refactors of the oscillator core (NEON/SoA, numOsc skip, polyphony) can be
// verified not to change the sound.
//
// To (re)generate the golden files after an *intentional* sound change:
//   ENOSC_GOLDEN_GENERATE=1 build/runtests -tc='*EnOsc golden*'
//
// Golden files are raw float32 LE, stereo interleaved, 48kHz. Listen with:
//   ffplay -f f32le -ar 48000 -ch_layout stereo enosc_golden/<name>.raw
// On mismatch, the actual output is written next to the golden file as
// <name>.actual.raw for A/B listening.

// Hack: include the .cc files directly (same approach as kpls/djembe tests):
#include "CoreModules/4ms/core/enosc/data.cc"
#include "CoreModules/4ms/core/enosc/dynamic_data.cc"
#include "CoreModules/4ms/core/EnOscCore.cc"

#include "doctest.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace
{

using MetaModule::EnOscCore;
using Info = MetaModule::EnOscInfo;
using CH = MetaModule::CoreHelper<Info>;
using Elem = Info::Elem;

constexpr float kSampleRate = 48000.f;
// Long enough for the slowest internal smoothing (the freeze/scale slew
// limiter takes 1024 blocks = 32768 samples to fully open) to settle:
constexpr unsigned kWarmupSamples = 96000;
constexpr unsigned kCaptureFrames = 8192;

// Switch positions (EnOscCore::switchstate): DOWN < 0.25 <= MID < 0.75 <= UP
constexpr float SW_UP = 1.f, SW_MID = 0.5f, SW_DOWN = 0.f;
// twist: UP=FEEDBACK MID=PULSAR DOWN=CRUSH
// warp:  UP=FOLD     MID=CHEBY  DOWN=SEGMENT
// mod:   UP=ONE      MID=TWO    DOWN=THREE
// scale: UP=TWELVE   MID=OCTAVE DOWN=FREE

struct Scenario {
	const char *name;

	float twist_sw = SW_UP, warp_sw = SW_UP, mod_sw = SW_MID, scale_sw = SW_MID;

	// knobs 0..1
	float scale_k = 0.f;
	float spread_k = 0.3f;
	float pitch_k = 0.5f;
	float balance_k = 0.5f;
	float root_k = 0.3f;
	float cross_fm_k = 0.3f;
	float twist_k = 0.5f;
	float detune_k = 0.2f;
	float warp_k = 0.55f;

	// alt params, normalized as in EnOscCore::set_param
	float freeze_split = 0.5f; // LOW_HIGH
	float stereo_split = 0.f;  // ALTERNATE
	float crossfade = 0.4f;
	float num_osc = 15.f / 31.f; // round(v*31)+1 => 16
	float fine_tune = 0.5f;

	// input jack voltages
	float pitch_cv = 0.f, root_cv = 0.f;
	bool freeze_gate = false;
};

// All-pairs would be 9 combos x everything; keep a representative set instead.
const Scenario scenarios[] = {
	// twist x warp matrix (mod mode TWO, numOsc=16, stereo ALTERNATE)
	{.name = "feedback_fold", .twist_sw = SW_UP, .warp_sw = SW_UP},
	{.name = "feedback_cheby", .twist_sw = SW_UP, .warp_sw = SW_MID},
	{.name = "feedback_segment", .twist_sw = SW_UP, .warp_sw = SW_DOWN},
	{.name = "pulsar_fold", .twist_sw = SW_MID, .warp_sw = SW_UP},
	{.name = "pulsar_cheby", .twist_sw = SW_MID, .warp_sw = SW_MID},
	{.name = "pulsar_segment", .twist_sw = SW_MID, .warp_sw = SW_DOWN},
	{.name = "crush_fold", .twist_sw = SW_DOWN, .warp_sw = SW_UP},
	{.name = "crush_cheby", .twist_sw = SW_DOWN, .warp_sw = SW_MID},
	{.name = "crush_segment", .twist_sw = SW_DOWN, .warp_sw = SW_DOWN},

	// cross-FM routing modes (ONE and THREE; TWO is covered above)
	{.name = "mod_one", .twist_sw = SW_MID, .warp_sw = SW_MID, .mod_sw = SW_UP, .cross_fm_k = 0.5f},
	{.name = "mod_three", .twist_sw = SW_MID, .warp_sw = SW_MID, .mod_sw = SW_DOWN, .cross_fm_k = 0.5f},
	// mode TWO with the cross-FM knob at zero (guards any future inert-chain fast path)
	{.name = "mod_two_off", .twist_sw = SW_MID, .warp_sw = SW_MID, .cross_fm_k = 0.f},

	// reduced oscillator counts (numOsc skip must not change these)
	{.name = "numosc_4", .num_osc = 3.f / 31.f},
	{.name = "numosc_7_lowhigh", .num_osc = 6.f / 31.f, .stereo_split = 0.5f},

	// lowest/rest stereo split with freeze latched on
	{.name = "lowest_rest_freeze", .twist_sw = SW_MID, .warp_sw = SW_UP, .stereo_split = 1.f, .freeze_gate = true},

	// 12-TET quantized scale, pitch & root CVs driven, wide spread
	{.name = "twelve_pitch_cv",
	 .twist_sw = SW_UP,
	 .warp_sw = SW_MID,
	 .scale_sw = SW_UP,
	 .scale_k = 0.4f,
	 .spread_k = 0.6f,
	 .pitch_cv = 2.f,
	 .root_cv = 1.f},
};

std::vector<float> run_scenario(Scenario const &s) {
	// The module (and the adaptor models inside it) has members that firmware
	// leaves uninitialized; zero the storage so runs are byte-deterministic.
	static std::vector<std::byte> storage;
	storage.assign(sizeof(EnOscCore), std::byte{});

	// Phasors draw their initial phase from this at construction:
	easiglib::Random::seed(0xE105C0DE);

	auto *core = new (storage.data()) EnOscCore();

	core->set_samplerate(kSampleRate);
	core->mark_all_inputs_unpatched();

	core->set_param(CH::param_idx<Elem::TwistSwitch>, s.twist_sw);
	core->set_param(CH::param_idx<Elem::WarpSwitch>, s.warp_sw);
	core->set_param(CH::param_idx<Elem::CrossFmSwitch>, s.mod_sw);
	core->set_param(CH::param_idx<Elem::ScaleSwitch>, s.scale_sw);

	core->set_param(CH::param_idx<Elem::ScaleKnob>, s.scale_k);
	core->set_param(CH::param_idx<Elem::SpreadKnob>, s.spread_k);
	core->set_param(CH::param_idx<Elem::PitchKnob>, s.pitch_k);
	core->set_param(CH::param_idx<Elem::BalanceKnob>, s.balance_k);
	core->set_param(CH::param_idx<Elem::RootKnob>, s.root_k);
	core->set_param(CH::param_idx<Elem::CrossFmKnob>, s.cross_fm_k);
	core->set_param(CH::param_idx<Elem::TwistKnob>, s.twist_k);
	core->set_param(CH::param_idx<Elem::DetuneKnob>, s.detune_k);
	core->set_param(CH::param_idx<Elem::WarpKnob>, s.warp_k);

	core->set_param(CH::param_idx<Elem::LearnButton>, 0.f);
	core->set_param(CH::param_idx<Elem::FreezeButton>, 0.f);

	core->set_param(CH::param_idx<Elem::FreezesplitAltParam>, s.freeze_split);
	core->set_param(CH::param_idx<Elem::StereosplitAltParam>, s.stereo_split);
	core->set_param(CH::param_idx<Elem::CrossfadeAltParam>, s.crossfade);
	core->set_param(CH::param_idx<Elem::NumoscAltParam>, s.num_osc);
	core->set_param(CH::param_idx<Elem::FinetuneAltParam>, s.fine_tune);

	core->set_input(Info::InputPitch_1V_Oct, s.pitch_cv);
	core->set_input(Info::InputRoot_1V_Oct, s.root_cv);
	if (s.freeze_gate)
		core->set_input(Info::InputFreeze_Cv, 5.f);

	for (unsigned i = 0; i < kWarmupSamples; i++)
		core->update();

	std::vector<float> capture(kCaptureFrames * 2);
	for (unsigned i = 0; i < kCaptureFrames; i++) {
		core->update();
		capture[2 * i + 0] = core->get_output(Info::OutputOut_A);
		capture[2 * i + 1] = core->get_output(Info::OutputOut_B);
	}

	core->~EnOscCore();
	return capture;
}

std::filesystem::path golden_dir() {
	return std::filesystem::path(__FILE__).parent_path() / "enosc_golden";
}

bool read_file(std::filesystem::path const &path, std::vector<float> &data) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;
	auto bytes = file.tellg();
	file.seekg(0);
	data.resize(size_t(bytes) / sizeof(float));
	file.read(reinterpret_cast<char *>(data.data()), data.size() * sizeof(float));
	return file.good();
}

void write_file(std::filesystem::path const &path, std::vector<float> const &data) {
	std::ofstream file(path, std::ios::binary);
	file.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(float));
}

struct DiffStats {
	float max_abs = 0.f;
	double rms = 0.;
	size_t first_idx = SIZE_MAX;
	size_t num_diffs = 0;
};

DiffStats compare(std::vector<float> const &golden, std::vector<float> const &actual) {
	DiffStats st;
	double sumsq = 0.;
	for (size_t i = 0; i < golden.size(); i++) {
		float d = std::abs(actual[i] - golden[i]);
		if (d > 0.f) {
			st.num_diffs++;
			if (st.first_idx == SIZE_MAX)
				st.first_idx = i;
		}
		st.max_abs = std::max(st.max_abs, d);
		sumsq += double(d) * double(d);
	}
	st.rms = std::sqrt(sumsq / double(golden.size()));
	return st;
}

} // namespace

TEST_CASE("EnOsc golden master") {
	const bool generate = std::getenv("ENOSC_GOLDEN_GENERATE") != nullptr;

	// Max absolute sample difference allowed vs golden (output is ~±9V full
	// scale). 0 = bit-exact; loosen deliberately if a refactor is *supposed*
	// to differ at float rounding level (and verify by ear first).
	const float tolerance = 0.f;

	auto dir = golden_dir();
	if (generate)
		std::filesystem::create_directories(dir);

	for (auto const &s : scenarios) {
		CAPTURE(std::string(s.name));
		auto actual = run_scenario(s);
		auto path = dir / (std::string(s.name) + ".raw");

		if (generate) {
			write_file(path, actual);
			MESSAGE("generated ", path.string());
			continue;
		}

		std::vector<float> golden;
		REQUIRE_MESSAGE(read_file(path, golden),
						"Missing golden file. Generate with: ENOSC_GOLDEN_GENERATE=1 runtests -tc='*EnOsc golden*'");
		REQUIRE(golden.size() == actual.size());

		auto st = compare(golden, actual);
		if (st.max_abs > tolerance) {
			auto actual_path = dir / (std::string(s.name) + ".actual.raw");
			write_file(actual_path, actual);
			MESSAGE("scenario '",
					s.name,
					"': max|diff|=",
					st.max_abs,
					" rms=",
					st.rms,
					" num_diffs=",
					st.num_diffs,
					"/",
					actual.size(),
					" first at sample ",
					st.first_idx / 2,
					" -- wrote ",
					actual_path.string());
		}
		CHECK_LE(st.max_abs, tolerance);
	}
}
