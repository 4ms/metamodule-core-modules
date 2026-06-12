#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Freeverb_info.hh"
#include "processors/allpass.h"
#include "processors/comb.h"
#include "processors/tools/dcBlock.h"
#include "util/math.hh"
#include "util/zip.hh"

#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: voice count follows the Input jack; each voice is an
// independent reverb. The CV inputs map per voice, with their highest
// channels feeding all upper voices.
class FreeverbCore : public SmartCoreProcessorPoly<FreeverbInfo> {
	using Info = FreeverbInfo;
	using ThisCore = FreeverbCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	FreeverbCore() {
		for (auto &voice : voices) {
			for (auto [comb, def] : zip(voice.combFilter, DefaultCombTuning)) {
				comb.setFeedback(0);
				comb.setLength(def);
			}

			for (auto [ap, def] : zip(voice.apFilter, DefaultAllPassTuning)) {
				ap.setLength(def);
				ap.setFeedback(0.6f);
				ap.setFadeSpeed(0.001f);
			}
		}
	}

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = std::max(numChannels<InputIn>(), 1u);
		setChannels<Out>(nv);

		auto add_cv_and_pot = [](float cv, float pot) {
			const float cv_val = cv / 5.f; // range: -1 .. 1 for CV -5V .. +5V
			return std::clamp(pot + cv_val, 0.f, 1.f);
		};

		const auto sizeKnob = getState<SizeKnob>();
		const auto dampKnob = getState<DampKnob>();
		const auto fbKnob = getState<FeedbackKnob>();
		const auto mixKnob = getState<MixKnob>();

		for (unsigned v = 0; v < nv; v++) {
			auto &voice = voices[v];

			if (auto size = add_cv_and_pot(getInputOrLast<SizeCvIn>(v), sizeKnob); voice.prev_size != size) {
				voice.prev_size = size;
				setSize(voice, size);
			}

			if (auto damp = add_cv_and_pot(getInputOrLast<DampCvIn>(v), dampKnob); voice.prev_damp != damp) {
				voice.prev_damp = damp;
				for (auto &comb : voice.combFilter)
					comb.setDamp(damp);
			}

			if (auto fb = add_cv_and_pot(getInputOrLast<FeedbackCvIn>(v), fbKnob); voice.prev_fb != fb) {
				voice.prev_fb = fb;
				for (auto &comb : voice.combFilter)
					comb.setFeedback(MathTools::map_value(fb, 0.0f, 1.0f, 0.8f, 1.0f));
			}

			float dry = getInput<InputIn>(v).value_or(0.f);
			float wet = 0;
			for (auto &comb : voice.combFilter) {
				wet += comb.process(dry);
			}

			wet /= static_cast<float>(NumComb);

			for (auto &allpass : voice.apFilter) {
				wet = allpass.process(wet);
			}

			const auto mix = add_cv_and_pot(getInputOrLast<MixCvIn>(v), mixKnob);

			setOutput<Out>(voice.dcblock.update(MathTools::interpolate3(dry, wet, mix)), v);
		}
	}

	void set_samplerate(float sr) override {
		sr_ratio = sr / 48000.f;
		for (auto &voice : voices)
			voice.prev_size = -1.f; //force re-calculate
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	static const int NumComb = 8;
	static const int NumAllPass = 4;

	static constexpr std::array<int, NumComb> DefaultCombTuning{1215, 1293, 1390, 1476, 1548, 1623, 1695, 1760};
	static constexpr std::array<int, NumAllPass> DefaultAllPassTuning{605, 480, 371, 245};

	static constexpr float MaxSize = 2.5f;
	static constexpr float MinSize = 0.23f;
	static constexpr size_t MaxCombSize = MaxSize * 1760 /* max(DefaultCombTuning) */;
	static constexpr size_t MaxAPSize = MaxSize * 605 /* max(DefaultAllPassTuning) */;

	struct Voice {
		std::array<Comb<MaxCombSize>, NumComb> combFilter{};
		std::array<AllPass<MaxAPSize>, NumAllPass> apFilter{};
		DCBlock dcblock;

		float prev_size{-1};
		float prev_damp{-1};
		float prev_fb{-1};
	};

	void setSize(Voice &voice, float val) {
		val *= (MaxSize - MinSize);
		val += MinSize;
		val *= sr_ratio;

		for (auto [comb, def] : zip(voice.combFilter, DefaultCombTuning)) {
			comb.setLength(std::clamp(def * val, 100.f, (float)MaxCombSize));
		}

		for (auto [ap, def] : zip(voice.apFilter, DefaultAllPassTuning)) {
			ap.setLength(std::clamp(def * val, 40.f, (float)MaxAPSize));
		}
	}

	// Read an input channel, reusing the input's highest channel when it has
	// fewer channels than requested. Returns 0 if unpatched.
	template<Info::Elem EL>
	float getInputOrLast(unsigned chan) {
		const auto chans = numChannels<EL>();
		if (chans == 0)
			return 0.f;
		return getInput<EL>(std::min(chan, chans - 1)).value_or(0.f);
	}

	std::array<Voice, MaxChans> voices{};

	float sr_ratio = 1.f;
};

} // namespace MetaModule
