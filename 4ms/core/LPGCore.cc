#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/LPG_info.hh"

#include "lpg/envelope.h"
#include "lpg/low_pass_gate.h"
#include "lpg/units.h"

#include "helpers/EdgeDetector.h"
#include "helpers/FlipFlop.h"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: voice count follows the Audio In jack; each voice has its own
// envelope and gate state. The Ping input and the CV inputs map per voice,
// with their highest channels feeding all upper voices.
class LPGCore : public SmartCoreProcessorPoly<LPGInfo> {
	using Info = LPGInfo;
	using ThisCore = LPGCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	LPGCore() {
		for (auto &l : lpg)
			l.Init();
		for (auto &e : envelope)
			e.Init();
	};

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = std::max(numChannels<AudioIn>(), 1u);
		setChannels<AudioOut>(nv);

		const bool pingPatched = isPatched<PingIn>();
		const auto colorKnob = getState<ColorKnob>();
		const auto decayKnob = getState<DecayKnob>();
		const auto levelKnob = getState<LevelKnob>();

		auto add_cv_and_pot = [](float cv, float pot) {
			const float cv_val = cv / 5.f; // range: -1 .. 1 for CV -5V .. +5V
			return std::clamp(pot + cv_val, 0.f, 1.f);
		};

		for (unsigned v = 0; v < nv; v++) {
			float in = getInput<AudioIn>(v).value_or(0.f);

			if (pingPatched) {
				if (pingEdge[v](pingIn[v](getInputOrLast<PingIn>(v)))) {
					envelope[v].Trigger();
				}
			}

			const auto lpg_colour = add_cv_and_pot(getInputOrLast<ColorCvIn>(v), colorKnob);
			const auto decay = add_cv_and_pot(getInputOrLast<DecayCvIn>(v), decayKnob);
			const float short_decay = 200.0f / sampleRate * LPG::stmlib::SemitonesToRatio(-96.0f * decay);
			const float decay_tail =
				20.0f / sampleRate * LPG::stmlib::SemitonesToRatio(-72.0f * decay + 12.0f * lpg_colour) - short_decay;

			if (pingPatched) {
				const float attack = (440.0f / 8.f) / sampleRate * 0.5f * LPG::stmlib::SemitonesToRatio(0.f);
				envelope[v].ProcessPing(attack, short_decay, decay_tail, lpg_colour);
			} else {
				const auto level = add_cv_and_pot(getInputOrLast<LevelCvIn>(v), levelKnob);
				envelope[v].ProcessLP(level, short_decay, decay_tail, lpg_colour);
			}

			lpg[v].Process(envelope[v].gain(), envelope[v].frequency(), envelope[v].hf_bleed(), &in, 1);

			setOutput<AudioOut>(in, v);
		}
	}

	void set_samplerate(float sr) override {
		sampleRate = sr;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// Read an input channel, reusing the input's highest channel when it has
	// fewer channels than requested. Returns 0 if unpatched.
	template<Info::Elem EL>
	float getInputOrLast(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 0.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(0.f);
	}

	float sampleRate = 48000.f;

	std::array<plaits::LowPassGate, MaxChans> lpg;
	std::array<plaits::LPGEnvelope, MaxChans> envelope;

	static_assert(MaxChans == 4, "FlipFlop initializer below assumes 4 channels");
	std::array<FlipFlop, MaxChans> pingIn{{{0.5f, 2.f}, {0.5f, 2.f}, {0.5f, 2.f}, {0.5f, 2.f}}};
	std::array<EdgeDetector, MaxChans> pingEdge{};
};

} // namespace MetaModule
