#include "CoreModules/moduleFactory.hh"
#include "helpers/poly_core_processor.hh"
#include "info/StMix_info.hh"

#include "util/math.hh"

namespace MetaModule
{

// Polyphonic: the output channel count is the highest channel count of all
// patched inputs (each input's highest channel feeds its upper voices). A
// channel's Right input normals to its Left input when unpatched.
class StMixCore : public PolyCoreProcessor<StMixInfo::NumInJacks, StMixInfo::NumOutJacks> {
	using Info = StMixInfo;
	using ThisCore = StMixCore;

public:
	StMixCore() = default;

	void update() override {
		auto &left = outs[Info::OutputLeft_Out];
		auto &right = outs[Info::OutputRight_Out];

		// Input jacks interleave Left/Right per mixer channel:
		static_assert(Info::InputCh__1_Left_In + 1 == Info::InputCh__1_Right_In);
		static_assert(Info::InputCh__1_Left_In + 2 == Info::InputCh__2_Left_In);
		static_assert(Info::InputCh__1_Left_In + 6 == Info::InputCh__4_Left_In);

		unsigned nv = 1;
		for (auto &jack : ins)
			nv = std::max<unsigned>(nv, jack.chans);
		nv = std::min(nv, MaxVoices);
		left.chans = nv;
		right.chans = nv;

		if (bypassed) {
			left.values = {};
			right.values = {};
			return;
		}

		for (unsigned v = 0; v < nv; v++) {
			float tempLeft = 0;
			float tempRight = 0;
			for (int i = 0; i < 4; i++) {
				float leftLevel;
				float rightLevel;
				if (pan[i] >= 0.5f) {
					leftLevel = level[i] * MathTools::map_value(pan[i], 0.5f, 1.0f, 1.0f, 0.0f);
					rightLevel = level[i];
				} else {
					leftLevel = level[i];
					rightLevel = level[i] * MathTools::map_value(pan[i], 0.0f, 0.5f, 0.0f, 1.0f);
				}

				auto &leftIn = ins[Info::InputCh__1_Left_In + 2 * i];
				auto &rightIn = ins[Info::InputCh__1_Right_In + 2 * i];
				float leftVal = leftIn.or_last(v);
				// Right input normals to the Left input when unpatched
				float rightVal = rightIn.is_patched() ? rightIn.or_last(v) : leftVal;

				tempLeft += leftVal * leftLevel;
				tempRight += rightVal * rightLevel;
			}

			left.values[v] = tempLeft;
			right.values[v] = tempRight;
		}
	}

	void set_param(int param_id, float val) override {
		switch (param_id) {
			case Info::KnobCh__1_Level:
				level[0] = val;
				break;
			case Info::KnobCh__2_Level:
				level[1] = val;
				break;
			case Info::KnobCh__3_Level:
				level[2] = val;
				break;
			case Info::KnobCh__4_Level:
				level[3] = val;
				break;
			case Info::KnobCh__1_Pan:
				pan[0] = val;
				break;
			case Info::KnobCh__2_Pan:
				pan[1] = val;
				break;
			case Info::KnobCh__3_Pan:
				pan[2] = val;
				break;
			case Info::KnobCh__4_Pan:
				pan[3] = val;
				break;
		}
	}

	float get_param(int param_id) const override {
		switch (param_id) {
			case Info::KnobCh__1_Level:
				return level[0];
			case Info::KnobCh__2_Level:
				return level[1];
			case Info::KnobCh__3_Level:
				return level[2];
			case Info::KnobCh__4_Level:
				return level[3];
			case Info::KnobCh__1_Pan:
				return pan[0];
			case Info::KnobCh__2_Pan:
				return pan[1];
			case Info::KnobCh__3_Pan:
				return pan[2];
			case Info::KnobCh__4_Pan:
				return pan[3];
		}
		return 0;
	}

	void set_samplerate(float sr) override {
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	float level[4]{1.f, 1.f, 1.f, 1.f};
	float pan[4]{0.5f, 0.5f, 0.5f, 0.5f};
};

} // namespace MetaModule
