#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/PEG_info.hh"
#include "patch-serial/base64.hh"

#include "peg/PEGChannel.h"

#include <algorithm>
#include <alpaca/alpaca.h>
#include <array>
#include <optional>

namespace MetaModule
{

class PEGCore : public SmartCoreProcessorPoly<PEGInfo> {
public:
	using Info = PEGInfo;
	using ThisCore = PEGCore;
	using enum Info::Elem;

	template<Info::Elem EL>
	void setOutput(auto val, unsigned voice = 0) {
		return SmartCoreProcessorPoly<Info>::setOutput<EL>(val, voice);
	}

	// Voice-mapped input: highest channel feeds upper voices; nullopt if unpatched
	template<Info::Elem EL>
	std::optional<float> getInput(unsigned voice = 0) {
		const auto chans = SmartCoreProcessorPoly<Info>::numChannels<EL>();
		if (chans == 0)
			return std::nullopt;
		return SmartCoreProcessorPoly<Info>::getInput<EL>(std::min(voice, chans - 1));
	}

	template<Info::Elem EL, typename VAL>
	void setLED(const VAL &value) {
		return SmartCoreProcessorPoly<Info>::setLED<EL>(value);
	}

	template<Info::Elem EL>
	auto getState() {
		return SmartCoreProcessorPoly<Info>::getState<EL>();
	}

private:
private:
	struct MappingA {
		const static Info::Elem PingDivMultKnob = PingDivMultRedKnob;
		const static Info::Elem ScaleKnob = ScaleRedKnob;
		const static Info::Elem SkewKnob = SkewRedKnob;
		const static Info::Elem CurveKnob = CurveRedKnob;
		const static Info::Elem PingButton = PingRedButton;
		const static Info::Elem CycleButton = CycleRedButton;
		const static Info::Elem BiNPolarButton = BiNPolarRedButton;
		const static Info::Elem PingJackIn = PingRedJackIn;
		const static Info::Elem QntIn = QntRedIn;
		const static Info::Elem AsyncIn = AsyncRedIn;
		const static Info::Elem DivJackIn = DivRedJackIn;
		const static Info::Elem SkewJackIn = SkewRedJackIn;
		const static Info::Elem CurveJackIn = CurveRedJackIn;
		const static Info::Elem EnvOut = EnvRedOut;
		const static Info::Elem _5VEnvOut = P5VEnvRedOut;
		const static Info::Elem EoFOut = EofRedOut;
		const static Info::Elem SecondaryOut = EorOut;
		const static Info::Elem EnvOutLight = EnvredLight;
		const static Info::Elem EofLight = EofredLight;
		const static Info::Elem SecondaryLight = EorredLight;

		const static Info::Elem AsyncModeAltParam = AsyncRedModeAltParam;
		const static Info::Elem FreeNRunningPingAltParam = FreeNRunningPingRedAltParam;
		const static Info::Elem SkewLimitAltParam = SkewLimitRedAltParam;
	};

	struct MappingB {
		const static Info::Elem PingDivMultKnob = PingDivMultBlueKnob;
		const static Info::Elem ScaleKnob = ScaleBlueKnob;
		const static Info::Elem SkewKnob = SkewBlueKnob;
		const static Info::Elem CurveKnob = CurveBlueKnob;
		const static Info::Elem PingButton = PingBlueButton;
		const static Info::Elem CycleButton = CycleBlueButton;
		const static Info::Elem BiNPolarButton = BiNPolarBlueButton;
		const static Info::Elem PingJackIn = PingBlueJackIn;
		const static Info::Elem QntIn = QntBlueIn;
		const static Info::Elem AsyncIn = AsyncBlueIn;
		const static Info::Elem DivJackIn = DivBlueJackIn;
		const static Info::Elem SkewJackIn = SkewBlueJackIn;
		const static Info::Elem CurveJackIn = CurveBlueJackIn;
		const static Info::Elem EnvOut = EnvBlueOut;
		const static Info::Elem _5VEnvOut = P5VEnvBlueOut;
		const static Info::Elem EoFOut = EofBlueOut;
		const static Info::Elem SecondaryOut = HalfROut;
		const static Info::Elem EnvOutLight = EnvblueLight;
		const static Info::Elem EofLight = EofblueLight;
		const static Info::Elem SecondaryLight = HalfriseblueLight;

		const static Info::Elem AsyncModeAltParam = AsyncBlueModeAltParam;
		const static Info::Elem FreeNRunningPingAltParam = FreeNRunningPingBlueAltParam;
		const static Info::Elem SkewLimitAltParam = SkewLimitBlueAltParam;
	};

	static constexpr unsigned MaxChans = MaxPolyChannels;

	// One PEGChannel per poly voice per side
	std::array<PEGChannel<PEGCore, MappingA>, MaxChans> channelA{{{this, 0}, {this, 1}, {this, 2}, {this, 3}}};
	std::array<PEGChannel<PEGCore, MappingB>, MaxChans> channelB{{{this, 0}, {this, 1}, {this, 2}, {this, 3}}};

	friend PEGChannel<PEGCore, MappingA>;
	friend PEGChannel<PEGCore, MappingB>;

	// Each side's voice count follows the widest of its Ping, QNT, and Async
	// jacks (the trigger/clock sources)
	unsigned numVoicesA() {
		return std::max({SmartCoreProcessorPoly<Info>::numChannels<PingRedJackIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<QntRedIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<AsyncRedIn>(),
						 1u});
	}

	unsigned numVoicesB() {
		return std::max({SmartCoreProcessorPoly<Info>::numChannels<PingBlueJackIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<QntBlueIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<AsyncBlueIn>(),
						 1u});
	}

public:
	PEGCore()
		: timerPhase(0)
		, timerPhaseIncrement(1.0f) {
	}

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nvA = numVoicesA();
		const unsigned nvB = numVoicesB();

		setChannels<EnvRedOut>(nvA);
		setChannels<P5VEnvRedOut>(nvA);
		setChannels<EofRedOut>(nvA);
		setChannels<EorOut>(nvA);
		setChannels<EnvBlueOut>(nvB);
		setChannels<P5VEnvBlueOut>(nvB);
		setChannels<EofBlueOut>(nvB);
		setChannels<HalfROut>(nvB);
		setChannels<OrOut>(std::max(nvA, nvB));

		timerPhase += timerPhaseIncrement;
		while (timerPhase > 1.0f) {
			for (unsigned v = 0; v < nvA; v++)
				channelA[v].doDACUpdate();
			for (unsigned v = 0; v < nvB; v++)
				channelB[v].doDACUpdate();
			timerPhase -= 1.0f;
		}

		for (unsigned v = 0; v < nvA; v++)
			channelA[v].update();
		for (unsigned v = 0; v < nvB; v++)
			channelB[v].update();

		if (isPatched<ToggleIn>()) {
			for (unsigned v = 0; v < nvA; v++)
				channelA[v].toggleInput(getInput<ToggleIn>(v));
			for (unsigned v = 0; v < nvB; v++)
				channelB[v].toggleInput(getInput<ToggleIn>(v));
		};

		using ChannelA_t = PEGChannel<PEGCore, MappingA>;
		using ChannelB_t = PEGChannel<PEGCore, MappingB>;

		static constexpr std::array<ChannelA_t::MainMode, 4> MainModesA{
			ChannelA_t::MainMode::EOF_GATE,
			ChannelA_t::MainMode::EOF_TRIG,
			ChannelA_t::MainMode::TAP_GATE,
			ChannelA_t::MainMode::TAP_TRIG};
		if (auto mode = getState<EofRedModeAltParam>(); mode < MainModesA.size())
			for (unsigned v = 0; v < nvA; v++)
				channelA[v].setMainMode(MainModesA[mode]);

		static constexpr std::array<ChannelB_t::MainMode, 4> MainModesB{
			ChannelB_t::MainMode::EOF_GATE,
			ChannelB_t::MainMode::EOF_TRIG,
			ChannelB_t::MainMode::TAP_GATE,
			ChannelB_t::MainMode::TAP_TRIG};
		if (auto mode = getState<EofBlueModeAltParam>(); mode < MainModesB.size())
			for (unsigned v = 0; v < nvB; v++)
				channelB[v].setMainMode(MainModesB[mode]);

		static constexpr std::array<ChannelA_t::SecondaryMode, 4> SecondaryModesA{
			ChannelA_t::SecondaryMode::EOR_GATE,
			ChannelA_t::SecondaryMode::EOR_TRIG,
			ChannelA_t::SecondaryMode::HR_GATE,
			ChannelA_t::SecondaryMode::HR_TRIG};
		if (auto mode = getState<EorRedModeAltParam>(); mode < SecondaryModesA.size())
			for (unsigned v = 0; v < nvA; v++)
				channelA[v].setSecondaryMode(SecondaryModesA[mode]);

		static constexpr std::array<ChannelB_t::SecondaryMode, 4> SecondaryModesB{
			ChannelB_t::SecondaryMode::HR_GATE,
			ChannelB_t::SecondaryMode::HR_TRIG,
			ChannelB_t::SecondaryMode::EOR_GATE,
			ChannelB_t::SecondaryMode::EOR_TRIG};
		if (auto mode = getState<HalfNRBlueModeAltParam>(); mode < SecondaryModesB.size())
			for (unsigned v = 0; v < nvB; v++)
				channelB[v].setSecondaryMode(SecondaryModesB[mode]);

		// Or Out: per-channel max of both sides' envelope outputs
		for (unsigned ch = 0; ch < std::max(nvA, nvB); ch++) {
			auto red = SmartCoreProcessorPoly<Info>::getOutput<EnvRedOut>(std::min(ch, nvA - 1));
			auto blue = SmartCoreProcessorPoly<Info>::getOutput<EnvBlueOut>(std::min(ch, nvB - 1));
			setOutput<OrOut>(std::max(red, blue), ch);
		}
	}

	void set_samplerate(float sr) override {
		// DAC update needs to happen at fixed rate, independent of sample rate
		timerPhaseIncrement = float(PEG::PEGBase::kDacSampleRate) / sr;
	}

	struct SaveState_t {
		bool cyclingA;
		bool cyclingB;
		uint32_t clk_timeA;
		uint32_t clk_timeB;
	};
	SaveState_t saveState;

	void load_state(std::string_view state_data) override {
		if (state_data.length() == 0) {
			saveState = SaveState_t{};

		} else {

			auto raw_data = Base64::decode(state_data);

			std::error_code ec;
			auto newSaveState = alpaca::deserialize<alpaca::options::with_version, SaveState_t>(raw_data, ec);
			if (ec)
				return;

			saveState = newSaveState;
		}

		for (auto &chan : channelA) {
			chan.peg.settings.start_clk_time = saveState.clk_timeA;
			chan.peg.settings.start_cycle_on = saveState.cyclingA;
			chan.peg.apply_settings();
		}

		for (auto &chan : channelB) {
			chan.peg.settings.start_clk_time = saveState.clk_timeB;
			chan.peg.settings.start_cycle_on = saveState.cyclingB;
			chan.peg.apply_settings();
		}
	}

	std::string save_state() override {
		saveState.cyclingA = channelA[0].peg.settings.start_cycle_on;
		saveState.clk_timeA = channelA[0].peg.settings.start_clk_time;

		saveState.cyclingB = channelB[0].peg.settings.start_cycle_on;
		saveState.clk_timeB = channelB[0].peg.settings.start_clk_time;

		std::vector<uint8_t> bytes;
		alpaca::serialize<alpaca::options::with_version>(saveState, bytes);
		return Base64::encode({bytes.data(), bytes.size()});
	}

private:
	float timerPhase;
	float timerPhaseIncrement;

public:
	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on
};

} // namespace MetaModule
