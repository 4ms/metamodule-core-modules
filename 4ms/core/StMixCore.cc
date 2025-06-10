#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/StMix_info.hh"

#include "util/math.hh"

namespace MetaModule
{

class StMixCore : public CoreProcessor {
	using Info = StMixInfo;
	using ThisCore = StMixCore;

public:
	StMixCore() = default;

	void update() override {
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
			tempLeft += leftInputs[i] * leftLevel;
			tempRight += rightInputs[i] * rightLevel;
		}

		leftOut = tempLeft;
		rightOut = tempRight;
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

	void set_input(int input_id, float val) override {
		switch (input_id) {
			case Info::InputCh__1_Left_In:
				leftInputs[0] = val;
				if (!rightConnected[0])
					rightInputs[0] = val;
				break;
			case Info::InputCh__2_Left_In:
				leftInputs[1] = val;
				if (!rightConnected[0])
					rightInputs[1] = val;
				break;
			case Info::InputCh__3_Left_In:
				leftInputs[2] = val;
				if (!rightConnected[0])
					rightInputs[2] = val;
				break;
			case Info::InputCh__4_Left_In:
				leftInputs[3] = val;
				if (!rightConnected[0])
					rightInputs[3] = val;
				break;
			case Info::InputCh__1_Right_In:
				if (rightConnected[0])
					rightInputs[0] = val;
				break;
			case Info::InputCh__2_Right_In:
				if (rightConnected[1])
					rightInputs[1] = val;
				break;
			case Info::InputCh__3_Right_In:
				if (rightConnected[2])
					rightInputs[2] = val;
				break;
			case Info::InputCh__4_Right_In:
				if (rightConnected[3])
					rightInputs[3] = val;
				break;
		}
	}

	float get_output(int output_id) const override {
		if (output_id == Info::OutputLeft_Out)
			return leftOut;
		if (output_id == Info::OutputRight_Out)
			return rightOut;
		return 0.f;
	}

	void set_samplerate(float sr) override {
	}

	float get_led_brightness(int led_id) const override {
		return 0.f;
	}

	void mark_input_unpatched(const int input_id) override {
		switch (input_id) {
			case Info::InputCh__1_Right_In:
				rightConnected[0] = false;
				break;
			case Info::InputCh__2_Right_In:
				rightConnected[1] = false;
				break;
			case Info::InputCh__3_Right_In:
				rightConnected[2] = false;
				break;
			case Info::InputCh__4_Right_In:
				rightConnected[3] = false;
				break;
		}
	}

	void mark_input_patched(const int input_id) override {
		switch (input_id) {
			case Info::InputCh__1_Right_In:
				rightConnected[0] = true;
				break;
			case Info::InputCh__2_Right_In:
				rightConnected[1] = true;
				break;
			case Info::InputCh__3_Right_In:
				rightConnected[2] = true;
				break;
			case Info::InputCh__4_Right_In:
				rightConnected[3] = true;
				break;
		}
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	float leftInputs[4]{0, 0, 0.0};
	float rightInputs[4]{0, 0, 0.0};
	float level[4]{1.f, 1.f, 1.f, 1.f};
	float pan[4]{0.5f, 0.5f, 0.5f, 0.5f};
	float leftOut = 0;
	float rightOut = 0;
	bool rightConnected[4]{false, false, false, false};
};

} // namespace MetaModule
