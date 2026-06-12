#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/VCAM_info.hh"
#include "vcam/Tables.h"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: the four outputs are as wide as the widest of the four signal
// inputs. Each input's highest channel feeds its upper voices, and the
// per-node control jacks map per voice the same way (defaulting to 5V when
// unpatched). Button LEDs show the first channel's gain.
class VCAMCore : public SmartCoreProcessorPoly<VCAMInfo> {
	using Info = VCAMInfo;
	using ThisCore = VCAMCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	VCAMCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = std::max(
			{numChannels<InAIn>(), numChannels<InBIn>(), numChannels<InCIn>(), numChannels<InDIn>(), 1u});

		setChannels<Out1Out>(nv);
		setChannels<Out2Out>(nv);
		setChannels<Out3Out>(nv);
		setChannels<Out4Out>(nv);

		std::array<std::array<float, MaxChans>, 4> outputs{};

		processRow<InAIn, A1LevelKnob, A1JackIn, A1Button, A2LevelKnob, A2JackIn, A2Button,
				   A3LevelKnob, A3JackIn, A3Button, A4LevelKnob, A4JackIn, A4Button>(nv, outputs);
		processRow<InBIn, B1LevelKnob, B1JackIn, B1Button, B2LevelKnob, B2JackIn, B2Button,
				   B3LevelKnob, B3JackIn, B3Button, B4LevelKnob, B4JackIn, B4Button>(nv, outputs);
		processRow<InCIn, C1LevelKnob, C1JackIn, C1Button, C2LevelKnob, C2JackIn, C2Button,
				   C3LevelKnob, C3JackIn, C3Button, C4LevelKnob, C4JackIn, C4Button>(nv, outputs);
		processRow<InDIn, D1LevelKnob, D1JackIn, D1Button, D2LevelKnob, D2JackIn, D2Button,
				   D3LevelKnob, D3JackIn, D3Button, D4LevelKnob, D4JackIn, D4Button>(nv, outputs);

		for (unsigned v = 0; v < nv; v++) {
			setOutput<Out1Out>(std::clamp(outputs[0][v], -10.f, 10.f), v);
			setOutput<Out2Out>(std::clamp(outputs[1][v], -10.f, 10.f), v);
			setOutput<Out3Out>(std::clamp(outputs[2][v], -10.f, 10.f), v);
			setOutput<Out4Out>(std::clamp(outputs[3][v], -10.f, 10.f), v);
		}
	}

	void set_samplerate(float sr) override {
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// One VCA node: gain = lookup(pot * control * mute), per voice
	template<Info::Elem PotE, Info::Elem CtrlE, Info::Elem ButtonE>
	void processNode(unsigned nv, bool inPatched, std::array<float, MaxChans> const &inVals,
					 std::array<float, MaxChans> &outAccum) {
		const float potMute = getState<PotE>() * (getState<ButtonE>() == LatchingButton::State_t::UP ? 0.f : 1.f);

		setLED<ButtonE>(VoltageToGainTable.lookup(potMute * controlOr5V<CtrlE>(0)));

		if (!inPatched)
			return;

		for (unsigned v = 0; v < nv; v++)
			outAccum[v] += inVals[v] * VoltageToGainTable.lookup(potMute * controlOr5V<CtrlE>(v));
	}

	template<Info::Elem InE,
			 Info::Elem P1, Info::Elem C1, Info::Elem B1,
			 Info::Elem P2, Info::Elem C2, Info::Elem B2,
			 Info::Elem P3, Info::Elem C3, Info::Elem B3,
			 Info::Elem P4, Info::Elem C4, Info::Elem B4>
	void processRow(unsigned nv, std::array<std::array<float, MaxChans>, 4> &outputs) {
		const bool inPatched = isPatched<InE>();
		std::array<float, MaxChans> inVals{};
		if (inPatched) {
			const auto chans = numChannels<InE>();
			for (unsigned v = 0; v < MaxChans; v++)
				inVals[v] = getInput<InE>(std::min(v, chans - 1)).value_or(0.f);
		}

		processNode<P1, C1, B1>(nv, inPatched, inVals, outputs[0]);
		processNode<P2, C2, B2>(nv, inPatched, inVals, outputs[1]);
		processNode<P3, C3, B3>(nv, inPatched, inVals, outputs[2]);
		processNode<P4, C4, B4>(nv, inPatched, inVals, outputs[3]);
	}

	// Control jack value per voice; defaults to 5V when unpatched
	template<Info::Elem EL>
	float controlOr5V(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 5.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(5.f);
	}
};

} // namespace MetaModule
