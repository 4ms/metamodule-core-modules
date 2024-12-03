#include "../../../../../src/medium/debug_raw.h"
#include "CoreModules/CoreHelper.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/STSP_info.hh"
#include "sampler/channel_mapping.hh"
#include "sampler/sampler_channel.hh"
#include "sampler/src/sts_filesystem.hh"
#include "sdcard.hh"
#include <atomic>
#include <chrono>

namespace MetaModule
{

class STSCore : public CoreProcessor {
public:
	using Info = STSPInfo;
	using ThisCore = STSCore;
	using enum Info::Elem;

private:
	// This runs in the low-pri thread:
	AsyncThread fs_thread{[this]() {
		if (!index_is_loaded) {
			printf("Loading samples from '%s'\n", root_dir.data());

			sd.reload_disk(root_dir);

			index_loader.load_all_banks();
			chanL.reset();
			chanR.reset();

			index_is_loaded = true;
		}

		if (tm - last_tm >= 1) {
			last_tm = tm;

			chanL.fs_process(tm);
			chanR.fs_process(tm);

			// For now, disable saving the index
			// index_loader.handle_events();
		}
	}};

public:
	void update() override {
		tm = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;

		if (index_is_loaded) {
			chanL.update(tm);
			chanR.update(tm);
		} else {
			//Leds leds{index_flags, controls};
			//leds.animate_startup()
		}

		if (!started_fs_thread && id > 0) {
			fs_thread.start(id);
			started_fs_thread = true;
		}
	}

	void set_param(int param_id, float val) override {
		settings.stereo_mode = false;

		if (param_id == CoreHelper<Info>::param_index<SampledirAltParam>()) {
			auto new_index_file = root_name(val);
			if (new_index_file != root_dir) {
				root_dir = new_index_file;
				// Disable live changing root
				// index_is_loaded = false;
			}
		}

		else if (chanL.set_param(param_id, val))
			return;
		else
			chanR.set_param(param_id, val);
	}

	float get_param(int param_id) const override {
		if (param_id == CoreHelper<Info>::param_index<SampledirAltParam>()) {
			if (root_dir == root_name(0))
				return 0;
			if (root_dir == root_name(0.25f))
				return 0.25f;
			if (root_dir == root_name(0.5f))
				return 0.5f;
			if (root_dir == root_name(0.75f))
				return 0.75f;
		}

		else if (auto val = chanL.get_param(param_id))
			return *val;

		else if (auto val = chanR.get_param(param_id))
			return *val;

		return 0;
	}

	void set_input(int input_id, float val) override {
		if (chanL.set_input(input_id, val))
			return;
		else
			chanR.set_input(input_id, val);
	}

	float get_output(int output_id) const override {
		//TODO: if chanR is not patched, feed mono to chan L

		if (output_id == OutL) {
			if (settings.stereo_mode)
				return 0.5f * (chanL.get_output(OutL).value_or(0.f) + chanR.get_output(OutL).value_or(0.f));
			else
				return chanL.get_output(OutL).value_or(0.f);

		} else if (output_id == OutR) {
			if (settings.stereo_mode)
				return 0.5f * (chanL.get_output(OutR).value_or(0.f) + chanR.get_output(OutR).value_or(0.f));
			else
				return chanR.get_output(OutL).value_or(0.f);

		} else if (auto found = chanL.get_output(output_id)) {
			return *found;

		} else if (auto found = chanR.get_output(output_id)) {
			return *found;
		}
		return 0.f;
	}

	void set_samplerate(float sr) override {
		chanL.set_samplerate(sr);
		chanR.set_samplerate(sr);
		ms_per_update = 1000.f / sr;
	}

	float get_led_brightness(int led_id) const override {
		if (auto found = chanL.get_led_brightness(led_id); found.has_value())
			return *found;

		else if (auto found = chanR.get_led_brightness(led_id); found.has_value())
			return *found;

		else if (led_id == CoreHelper<Info>::first_light_index<BusyLight>())
			return index_is_loaded ? 0 : 1.f;

		else
			return 0.f;
	}

	size_t get_display_text(int led_id, std::span<char> text) override {
		std::string chars = "Stereo Triggered Sample Player";

		size_t chars_to_copy = std::min(text.size(), chars.length());
		std::copy(chars.begin(), std::next(chars.begin(), chars_to_copy), text.begin());

		return chars_to_copy;
	}

	std::string_view root_name(float val) const {
		unsigned index = std::clamp<unsigned>(std::round(val * 4.f), 0, 3);
		return sample_root_dirs[index];
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	SamplerKit::Sdcard sd;
	SamplerKit::SampleList samples;
	SamplerKit::BankManager banks{samples};
	SamplerKit::UserSettings settings{}; //TODO: load from file
	SamplerKit::CalibrationStorage cal_storage;

	uint32_t tm = 0;
	uint32_t last_tm = 0;
	float ms_per_update = 1000.f / 48000.f;
	bool started_fs_thread = false;

	// TODO: outputs for each side, normalized to left side
	constexpr static uint8_t OutL = CoreHelper<Info>::output_index<OutALeftOut>();
	constexpr static uint8_t OutR = CoreHelper<Info>::output_index<OutALeftOut>();

	constexpr static STSChanMapping MappingL{
		.PitchKnob = CoreHelper<Info>::param_index<PitchAKnob>(),
		.SampleKnob = CoreHelper<Info>::param_index<SampleAKnob>(),
		.StartPosKnob = CoreHelper<Info>::param_index<StartPos_AKnob>(),
		.LengthKnob = CoreHelper<Info>::param_index<LengthAKnob>(),
		.PlayButton = CoreHelper<Info>::param_index<PlayAButton>(),
		.BankPrevButton = CoreHelper<Info>::param_index<PrevBankAButton>(),
		.BankNextButton = CoreHelper<Info>::param_index<NextBankAButton>(),
		.ReverseButton = CoreHelper<Info>::param_index<ReverseAButton>(),
		.PlayTrigIn = CoreHelper<Info>::input_index<PlayTrigAIn>(),
		.VOctIn = CoreHelper<Info>::input_index<PitchV_OctAIn>(),
		.ReverseTrigIn = CoreHelper<Info>::input_index<ReverseTrigAIn>(),
		.LengthCvIn = CoreHelper<Info>::input_index<LengthCvAIn>(),
		.StartPosCvIn = CoreHelper<Info>::input_index<StartPosCvAIn>(),
		.SampleCvIn = CoreHelper<Info>::input_index<SampleCvAIn>(),
		.OutL = CoreHelper<Info>::output_index<OutALeftOut>(),
		.OutR = CoreHelper<Info>::output_index<OutALeftOut>(),
		.EndOut = CoreHelper<Info>::output_index<EndOutAOut>(),
		.PlayLight = CoreHelper<Info>::first_light_index<PlayALight>(),
		.PlayButR = CoreHelper<Info>::first_light_index<PlayAButton>() + 0,
		.PlayButG = CoreHelper<Info>::first_light_index<PlayAButton>() + 1,
		.PlayButB = CoreHelper<Info>::first_light_index<PlayAButton>() + 2,
		.RevButR = CoreHelper<Info>::first_light_index<ReverseAButton>() + 0,
		.RevButG = CoreHelper<Info>::first_light_index<ReverseAButton>() + 1,
		.RevButB = CoreHelper<Info>::first_light_index<ReverseAButton>() + 2,
		.BankR = CoreHelper<Info>::first_light_index<BankALight>() + 0,
		.BankG = CoreHelper<Info>::first_light_index<BankALight>() + 1,
		.BankB = CoreHelper<Info>::first_light_index<BankALight>() + 2,
	};

	constexpr static STSChanMapping MappingR{
		.PitchKnob = CoreHelper<Info>::param_index<PitchBKnob>(),
		.SampleKnob = CoreHelper<Info>::param_index<SampleBKnob>(),
		.StartPosKnob = CoreHelper<Info>::param_index<StartPos_BKnob>(),
		.LengthKnob = CoreHelper<Info>::param_index<LengthBKnob>(),
		.PlayButton = CoreHelper<Info>::param_index<PlayBButton>(),
		.BankPrevButton = CoreHelper<Info>::param_index<PrevBankBButton>(),
		.BankNextButton = CoreHelper<Info>::param_index<NextBankBButton>(),
		.ReverseButton = CoreHelper<Info>::param_index<ReverseBButton>(),
		.PlayTrigIn = CoreHelper<Info>::input_index<PlayTrigBIn>(),
		.VOctIn = CoreHelper<Info>::input_index<PitchV_OctBIn>(),
		.ReverseTrigIn = CoreHelper<Info>::input_index<ReverseTrigBIn>(),
		.LengthCvIn = CoreHelper<Info>::input_index<LengthCvBIn>(),
		.StartPosCvIn = CoreHelper<Info>::input_index<StartPosCvBIn>(),
		.SampleCvIn = CoreHelper<Info>::input_index<SampleCvBIn>(),
		.OutL = CoreHelper<Info>::output_index<OutBLeftOut>(),
		.OutR = CoreHelper<Info>::output_index<OutBLeftOut>(),
		.EndOut = CoreHelper<Info>::output_index<EndOutBOut>(),
		.PlayLight = CoreHelper<Info>::first_light_index<PlayBLight>(),
		.PlayButR = CoreHelper<Info>::first_light_index<PlayBButton>() + 0,
		.PlayButG = CoreHelper<Info>::first_light_index<PlayBButton>() + 1,
		.PlayButB = CoreHelper<Info>::first_light_index<PlayBButton>() + 2,
		.RevButR = CoreHelper<Info>::first_light_index<ReverseBButton>() + 0,
		.RevButG = CoreHelper<Info>::first_light_index<ReverseBButton>() + 1,
		.RevButB = CoreHelper<Info>::first_light_index<ReverseBButton>() + 2,
		.BankR = CoreHelper<Info>::first_light_index<BankBLight>() + 0,
		.BankG = CoreHelper<Info>::first_light_index<BankBLight>() + 1,
		.BankB = CoreHelper<Info>::first_light_index<BankBLight>() + 2,
	};

	SamplerChannel chanL{MappingL, sd, banks, settings, cal_storage};
	SamplerChannel chanR{MappingR, sd, banks, settings, cal_storage};

	SamplerKit::Flags index_flags;
	std::atomic<bool> index_is_loaded = false;
	SamplerKit::SampleIndexLoader index_loader{sd, samples, banks, index_flags};

	static constexpr std::array<std::string_view, 4> sample_root_dirs = {
		"",
		"Samples-1",
		"Samples-2",
		"Samples-3",
	};
	std::string_view root_dir = sample_root_dirs[0];
};

} // namespace MetaModule
