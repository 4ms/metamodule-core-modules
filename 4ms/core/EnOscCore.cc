#include "CoreModules/CoreHelper.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/EnOsc_info.hh"

#include "enosc/easiglib/dsp.hh"
using namespace easiglib;
#include "enosc/ui.hh"

namespace MetaModule
{

class EnOscCore : public CoreProcessor, CoreHelper<EnOscInfo> {
	using Info = EnOscInfo;
	using ThisCore = EnOscCore;

	enum { kUiUpdateRate = 60 };
	enum { kUiProcessRate = 20 };
	enum { kUiPollRate = 6000 };

public:
	EnOscCore() = default;

	void update() override {
		// Low-priority thread
		// in while loop:
		if (ui_process_ctr++ > ui_process_throttle) {
			ui_process_ctr = 0;
			enosc.Process(); //EventHandler::Process
		}

		if (ui_update_ctr++ > ui_update_throttle) {
			ui_update_ctr = 0;
			enosc.Update(); //LED update
		}

		if (ui_poll_ctr++ > ui_poll_throttle) {
			ui_poll_ctr = 0;
			enosc.Poll();
		}

		// SampleRate / BlockRate (6kHz for 48k)
		if (++block_ctr >= EnOsc::kBlockSize) {
			block_ctr = 0;
			enosc.osc().Process(out_block_);
		}
	}

	// DOWN=0 / MID=0.5 / UP=1.0
	EnOsc::Switches::State switchstate(float val) {
		using enum EnOsc::Switches::State;
		return val < 0.25f ? DOWN : val < 0.75f ? MID : UP;
	}

	static float switchstate_to_float(EnOsc::Switches::State state) {
		using enum EnOsc::Switches::State;
		return state == DOWN ? 0.f : state == MID ? .5f : 1.f;
	}

	void set_param(int param_id, float val) override {
		using AdcInput = EnOsc::AdcInput;

		switch (param_id) {
			// Knobs
			case param_index<Elem::ScaleKnob>():
				enosc.set_potcv(AdcInput::POT_SCALE, val);
				break;
			case param_index<Elem::SpreadKnob>():
				enosc.set_potcv(AdcInput::POT_SPREAD, val);
				break;
			case param_index<Elem::PitchKnob>():
				enosc.set_potcv(AdcInput::POT_PITCH, val);
				break;
			case param_index<Elem::BalanceKnob>():
				enosc.set_potcv(AdcInput::POT_BALANCE, val);
				break;
			case param_index<Elem::RootKnob>():
				enosc.set_potcv(AdcInput::POT_ROOT, val);
				break;
			case param_index<Elem::CrossFmKnob>():
				enosc.set_potcv(AdcInput::POT_MOD, val);
				break;
			case param_index<Elem::TwistKnob>():
				enosc.set_potcv(AdcInput::POT_TWIST, val);
				break;
			case param_index<Elem::DetuneKnob>():
				enosc.set_potcv(AdcInput::POT_DETUNE, val);
				break;
			case param_index<Elem::WarpKnob>():
				enosc.set_potcv(AdcInput::POT_WARP, val);
				break;

			// Switches
			case param_index<Elem::ScaleSwitch>():
				enosc.switches().scale_.set(switchstate(val));
				break;
			case param_index<Elem::CrossFmSwitch>():
				enosc.switches().mod_.set(switchstate(val));
				break;
			case param_index<Elem::TwistSwitch>():
				enosc.switches().twist_.set(switchstate(val));
				break;
			case param_index<Elem::WarpSwitch>():
				enosc.switches().warp_.set(switchstate(val));
				break;
			case param_index<Elem::LearnButton>():
				enosc.set_learn_button(val > 0.5f);
				break;
			case param_index<Elem::FreezeButton>():
				enosc.set_freeze_button(val > 0.5f);
				break;

			// AltParams
			case param_index<Elem::FreezesplitAltParam>(): {
				auto mode = static_cast<EnOsc::SplitMode>(std::round(val * 2.f));
				enosc.set_freeze_mode(mode);
			} break;

			case param_index<Elem::StereosplitAltParam>(): {
				auto mode = static_cast<EnOsc::SplitMode>(std::round(val * 2.f));
				enosc.set_stereo_mode(mode);
			} break;

			case param_index<Elem::CrossfadeAltParam>():
				enosc.set_crossfade(val);
				break;

			case param_index<Elem::NumoscAltParam>():
				enosc.set_num_osc(std::round(val * 15.f) + 1);
				break;

			case param_index<Elem::FinetuneAltParam>():
				enosc.set_fine_tune(val);
				break;
		}
	}

	float get_param(int param_id) const override {
		using AdcInput = EnOsc::AdcInput;

		switch (param_id) {
			// Knobs
			case param_index<Elem::ScaleKnob>():
				return std::round(enosc.get_potcv(AdcInput::POT_SCALE) * 9.f) / 9.f;
			case param_index<Elem::SpreadKnob>():
				return enosc.get_potcv(AdcInput::POT_SPREAD);
			case param_index<Elem::PitchKnob>():
				return enosc.get_potcv(AdcInput::POT_PITCH);
			case param_index<Elem::BalanceKnob>():
				return enosc.get_potcv(AdcInput::POT_BALANCE);
			case param_index<Elem::RootKnob>():
				return enosc.get_potcv(AdcInput::POT_ROOT);
			case param_index<Elem::CrossFmKnob>():
				return enosc.get_potcv(AdcInput::POT_MOD);
			case param_index<Elem::TwistKnob>():
				return enosc.get_potcv(AdcInput::POT_TWIST);
			case param_index<Elem::DetuneKnob>():
				return enosc.get_potcv(AdcInput::POT_DETUNE);
			case param_index<Elem::WarpKnob>():
				return enosc.get_potcv(AdcInput::POT_WARP);

			// Switches
			case param_index<Elem::ScaleSwitch>():
				return switchstate_to_float(enosc.switches().scale_.get());
			case param_index<Elem::CrossFmSwitch>():
				return switchstate_to_float(enosc.switches().mod_.get());
			case param_index<Elem::TwistSwitch>():
				return switchstate_to_float(enosc.switches().twist_.get());
			case param_index<Elem::WarpSwitch>():
				return switchstate_to_float(enosc.switches().warp_.get());
			case param_index<Elem::LearnButton>():
				return enosc.get_learn_button() ? 0.f : 1.f;
			case param_index<Elem::FreezeButton>():
				return enosc.get_freeze_button() ? 0.f : 1.f;

			// AltParams
			case param_index<Elem::FreezesplitAltParam>(): {
				auto mode = static_cast<float>(enosc.get_freeze_mode());
				return std::clamp(mode / 2.f, 0.f, 1.f);
			} break;

			case param_index<Elem::StereosplitAltParam>(): {
				auto mode = static_cast<float>(enosc.get_stereo_mode());
				return std::clamp(mode / 2.f, 0.f, 1.f);
			} break;

			case param_index<Elem::CrossfadeAltParam>():
				return enosc.get_crossfade();

			case param_index<Elem::NumoscAltParam>():
				return (enosc.get_num_osc() - 1) / 15.f;

			case param_index<Elem::FinetuneAltParam>():
				return enosc.get_fine_tune();

			default:
				return 0;
		}
	}

	void set_input(int input_id, float cv) override {
		using AdcInput = EnOsc::AdcInput;
		using SpiAdcInput = EnOsc::SpiAdcInput;

		auto cv_to_val = [](float cv) {
			float val = cv / 5.f; // -5V to +5V => -1..1
			val *= -0.5f;		  // -1..1 => 0.5..-0.5
			val += 0.5f;		  // => 1..0
			return val;
		};

		auto pitchcv_to_val = [](float cv) {
			float val = cv / 8.f; // -8V to +8V => -1..1
			val *= -0.5f;		  // -1..1 => 0.5..-0.5
			val += 0.5f;		  // => 1..0
			return val;
		};

		constexpr float gate_threshold = 1.5f;

		// Ui::set_potcv will clamp
		switch (input_id) {
			case Info::InputBalance_Cv:
				enosc.set_potcv(AdcInput::CV_BALANCE, cv_to_val(cv));
				break;
			case Info::InputCross_Fm_Cv:
				enosc.set_potcv(AdcInput::CV_MOD, cv_to_val(cv));
				break;
			case Info::InputPitch_1V_Oct:
				enosc.set_pitchroot_cv(SpiAdcInput::CV_PITCH, pitchcv_to_val(cv));
				break;
			case Info::InputRoot_1V_Oct:
				enosc.set_pitchroot_cv(SpiAdcInput::CV_ROOT, pitchcv_to_val(cv));
				break;
			case Info::InputScale_Cv:
				enosc.set_potcv(AdcInput::CV_SCALE, cv_to_val(cv));
				break;
			case Info::InputSpread_Cv:
				enosc.set_potcv(AdcInput::CV_SPREAD, cv_to_val(cv));
				break;
			case Info::InputTwist_Cv:
				enosc.set_potcv(AdcInput::CV_TWIST, cv_to_val(cv));
				break;
			case Info::InputWarp_Cv:
				enosc.set_potcv(AdcInput::CV_WARP, cv_to_val(cv));
				break;
			case Info::InputFreeze_Cv:
				enosc.set_freeze_gate(cv > gate_threshold);
				break;
			case Info::InputLearn_Cv:
				enosc.set_learn_gate(cv > gate_threshold);
				break;
		}
	}

	float get_output(int output_id) const override {
		s9_23 sample = output_id == 0 ? out_block_[block_ctr].l : out_block_[block_ctr].r;

		//hardware EnOssc is about 4.5Vpp for one osc, we make it 2x as loud to match other virtual VCOs
		auto s = f::inclusive(sample).repr() * 9.f;
		return s;
	}

	void set_samplerate(float sr) override {
		if (sample_rate_ != sr) {
			sample_rate_ = sr;
			ui_process_throttle = (unsigned)sample_rate_ / kUiProcessRate;
			ui_update_throttle = (unsigned)sample_rate_ / kUiUpdateRate;
			ui_poll_throttle = (unsigned)sample_rate_ / kUiPollRate;

			enosc.set_samplerate(sr);
		}
	}

	float get_led_brightness(int led_id) const override {
		if (led_id < 0 || led_id > 5)
			return 0.f;

		if (led_id < 3) {
			auto c = enosc.get_learn_led_color();
			auto el = led_id == 0 ? c.red() : led_id == 1 ? c.green() : c.blue();
			return f::inclusive(el).repr();
		} else {
			auto c = enosc.get_freeze_led_color();
			auto el = led_id == 3 ? c.red() : led_id == 4 ? c.green() : c.blue();
			return f::inclusive(el).repr();
		}
	}

	void mark_all_inputs_unpatched() override {
		for (unsigned i = 0; i < Info::NumInJacks; i++)
			set_input(i, 0.f);
	}

	void mark_input_unpatched(const int input_id) override {
		set_input(input_id, 0.f);
	}

	void mark_input_patched(const int input_id) override {
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	EnOsc::Ui<kUiUpdateRate, EnOsc::kBlockSize> enosc;
	Buffer<EnOsc::Frame, EnOsc::kBlockSize> out_block_;
	EnOsc::DynamicData dydata;

	float sample_rate_ = 48000.f;

	unsigned ui_process_throttle = (unsigned)sample_rate_ / kUiProcessRate;
	unsigned ui_process_ctr = ui_process_throttle;

	unsigned ui_update_throttle = (unsigned)sample_rate_ / kUiUpdateRate;
	unsigned ui_update_ctr = ui_update_throttle;

	unsigned ui_poll_throttle = (unsigned)sample_rate_ / kUiPollRate;
	unsigned ui_poll_ctr = ui_poll_throttle;

	unsigned block_ctr = EnOsc::kBlockSize;
};

} // namespace MetaModule
