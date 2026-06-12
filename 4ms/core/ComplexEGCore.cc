#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/ComplexEG_info.hh"
#include "processors/envelope.h"
#include "util/math.hh"

#include <algorithm>
#include <array>
#include <optional>

using namespace MathTools;

namespace MetaModule
{

// Polyphonic: voice count follows the Gate In jack; each voice has its own
// envelope. The CV inputs map per voice, with their highest channels feeding
// all upper voices. All outputs carry one channel per voice.
class ComplexEGCore : public SmartCoreProcessorPoly<ComplexEGInfo> {
	using Info = ComplexEGInfo;
	using ThisCore = ComplexEGCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	ComplexEGCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = std::max(numChannels<GateIn>(), 1u);
		setChannels<EnvOut>(nv);
		setChannels<AttackOut>(nv);
		setChannels<HoldOut>(nv);
		setChannels<DecayOut>(nv);
		setChannels<SustainOut>(nv);
		setChannels<ReleaseOut>(nv);

		const bool isLooping = getState<LoopSwitch>() == 1 ? true : false;
		const auto attackKnob = getState<AttackKnob>();
		const auto holdKnob = getState<HoldKnob>();
		const auto decayKnob = getState<DecayKnob>();
		const auto sustainKnob = getState<SustainKnob>();
		const auto releaseKnob = getState<ReleaseKnob>();
		const auto attackCurve = getState<AttackCurveKnob>();
		const auto decayCurve = getState<DecayCurveKnob>();
		const auto releaseCurve = getState<ReleaseCurveKnob>();

		for (unsigned v = 0; v < nv; v++) {
			auto &env = e[v];

			float finalAttack = constrain(getInputOrLast<AttackCvIn>(v) / CvRangeVolts + attackKnob, 0.0f, 1.0f);
			float finalHold = constrain(getInputOrLast<HoldCvIn>(v) / CvRangeVolts + holdKnob, 0.0f, 1.0f);
			float finalDecay = constrain(getInputOrLast<DecayCvIn>(v) / CvRangeVolts + decayKnob, 0.0f, 1.0f);
			float finalSustain = constrain(getInputOrLast<SustainCvIn>(v) / CvRangeVolts + sustainKnob, 0.0f, 1.0f);
			float finalRelease = constrain(getInputOrLast<ReleaseCvIn>(v) / CvRangeVolts + releaseKnob, 0.0f, 1.0f);

			env.set_envelope_time(env.ATTACK, map_value(finalAttack, 0.0f, 1.0f, 0.1f, 1000.0f));
			env.set_envelope_time(env.HOLD, map_value(finalHold, 0.0f, 1.0f, 0.0f, 1000.0f));
			env.set_envelope_time(env.DECAY, map_value(finalDecay, 0.0f, 1.0f, 0.1f, 1000.0f));
			env.set_envelope_time(env.RELEASE, map_value(finalRelease, 0.0f, 1.0f, 0.1f, 1000.0f));

			env.set_sustain(finalSustain);

			env.set_attack_curve(attackCurve);
			env.set_decay_curve(decayCurve);
			env.set_release_curve(releaseCurve);

			float envelopeOutput;
			if (isLooping) {
				if (currentStage[v] == Envelope::IDLE) {
					envelopeOutput = env.update(8.f);
				} else {
					envelopeOutput = env.update(0.f);
				}
			} else {
				envelopeOutput = env.update(getInput<GateIn>(v).value_or(0.f));
			}

			currentStage[v] = env.getStage();

			setOutput<AttackOut>((currentStage[v] == env.ATTACK) ? MaxOutputVolts : 0, v);
			setOutput<HoldOut>((currentStage[v] == env.HOLD) ? MaxOutputVolts : 0, v);
			setOutput<DecayOut>((currentStage[v] == env.DECAY) ? MaxOutputVolts : 0, v);
			setOutput<SustainOut>((currentStage[v] == env.SUSTAIN) ? MaxOutputVolts : 0, v);
			setOutput<ReleaseOut>((currentStage[v] == env.RELEASE) ? MaxOutputVolts : 0, v);

			setOutput<EnvOut>(envelopeOutput * MaxOutputVolts, v);
		}
	}

	void set_samplerate(float sr) override {
		for (auto &env : e)
			env.set_samplerate(sr);
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

	std::array<Envelope::stage_t, MaxChans> currentStage{
		Envelope::stage_t::ATTACK, Envelope::stage_t::ATTACK, Envelope::stage_t::ATTACK, Envelope::stage_t::ATTACK};
	std::array<Envelope, MaxChans> e{};
};

} // namespace MetaModule
