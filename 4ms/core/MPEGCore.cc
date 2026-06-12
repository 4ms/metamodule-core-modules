#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/MPEG_info.hh"
#include "mpeg/envelope_calcs.hh"
#include "patch-serial/base64.hh"
#include "peg-common/peg_base.hh"

using namespace MetaModule::PEG;
#include "helpers/EdgeDetector.h"
#include "helpers/FlipFlop.h"

#include <algorithm>
#include <alpaca/alpaca.h>
#include <array>
#include <optional>

namespace MetaModule
{

// Polyphonic: voice count follows the widest of the Ping, Trigger, and Cycle
// jacks; each voice has its own envelope core. The CV inputs map per voice,
// with their highest channels feeding all upper voices. LEDs show voice 0.
class MPEGCore : public SmartCoreProcessorPoly<MPEGInfo> {
	using Info = MPEGInfo;
	using ThisCore = MPEGCore;
	using enum Info::Elem;

	static constexpr unsigned MaxChans = MaxPolyChannels;

public:
	MPEGCore() {
		for (unsigned v = 0; v < MaxChans; v++) {
			// TODO: maybe calling these is not required
			sideloadDrivers(v);
			sideloadSystemSettings(v);
			voices[v].peg.apply_settings();
			voices[v].peg.update_all_envelopes();
		}
	};

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const unsigned nv = numVoices();
		setChannels<EofOut>(nv);
		setChannels<EnvOut>(nv);
		setChannels<_5VEnvOut>(nv);

		for (unsigned v = 0; v < nv; v++) {
			sideloadDrivers(v);
			sideloadSystemSettings(v);
		}

		timerPhase += timerPhaseIncrement;
		while (timerPhase > 1.0f) {
			for (unsigned v = 0; v < nv; v++)
				voices[v].peg.update_all_envelopes();
			timerPhase -= 1.0f;
		}

		for (unsigned v = 0; v < nv; v++)
			voices[v].peg.update();
	}

	void set_samplerate(float sr) override {
		timerPhaseIncrement = float(PEG::PEGBase::kDacSampleRate) / sr;
	}

	struct SaveState_t {
		bool cycling;
		uint32_t clk_time;
	};
	SaveState_t saveState;

	void load_state(std::string_view state_data) override {
		if (state_data.length() == 0) {
			saveState = SaveState_t{};
			for (auto &voice : voices)
				voice.peg.apply_settings();
			return;
		}

		auto raw_data = Base64::decode(state_data);

		std::error_code ec;
		auto newSaveState = alpaca::deserialize<alpaca::options::with_version, SaveState_t>(raw_data, ec);
		if (!ec) {
			saveState = newSaveState;

			for (auto &voice : voices) {
				voice.peg.settings.start_clk_time = saveState.clk_time;
				voice.peg.settings.start_cycle_on = saveState.cycling;
				voice.peg.apply_settings();
			}
		}
	}

	std::string save_state() override {
		saveState.cycling = voices[0].peg.settings.start_cycle_on;
		saveState.clk_time = voices[0].peg.settings.start_clk_time;

		std::vector<uint8_t> bytes;
		alpaca::serialize<alpaca::options::with_version>(saveState, bytes);
		return Base64::encode({bytes.data(), bytes.size()});
	}

private:
	unsigned numVoices() {
		return std::max({SmartCoreProcessorPoly<Info>::numChannels<PingJackIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<TriggerIn>(),
						 SmartCoreProcessorPoly<Info>::numChannels<CycleTrigIn>(),
						 1u});
	}

	// Voice-mapped input: highest channel feeds upper voices; nullopt if unpatched
	template<Info::Elem EL>
	std::optional<float> getInput(unsigned voice) {
		const auto chans = SmartCoreProcessorPoly<Info>::numChannels<EL>();
		if (chans == 0)
			return std::nullopt;
		return SmartCoreProcessorPoly<Info>::getInput<EL>(std::min(voice, chans - 1));
	}

	void sideloadDrivers(unsigned v) {
		auto &voice = voices[v];
		auto &peg = voice.peg;

		auto MapOutputFunc = [](auto val) -> uint16_t {
			auto result = val / CVInputFullScaleInV + 0.5f;
			return uint16_t(std::clamp(result, 0.f, 1.f) * 4095.f);
		};

		peg.adc_dma_buffer[CV_SHAPE] = MapOutputFunc(getInput<ShapeCvIn>(v).value_or(0.f));
		peg.adc_dma_buffer[CV_DIVMULT] = MapOutputFunc(getInput<Div_MultCvIn>(v).value_or(0.f));

		auto MapKnobFunc = [](auto val) -> uint16_t {
			return uint16_t(val * 4095.f);
		};

		peg.adc_dma_buffer[POT_SCALE] = MapKnobFunc(getState<ScaleKnob>());
		peg.adc_dma_buffer[POT_OFFSET] = MapKnobFunc(getState<OffsetKnob>());
		peg.adc_dma_buffer[POT_SHAPE] = MapKnobFunc(getState<ShapeKnob>());
		peg.adc_dma_buffer[POT_DIVMULT] = MapKnobFunc(getState<Div_MultKnob>());

		peg.digio.PingBut.sideload_set(getState<PingButton>() == MomentaryButton::State_t::PRESSED);
		peg.digio.CycleBut.sideload_set(getState<CycleButton>() == MomentaryButton::State_t::PRESSED);

		peg.digio.CycleJack.sideload_set(voice.cycleIn(getInput<CycleTrigIn>(v).value_or(0.f)));
		peg.digio.TrigJack.sideload_set(voice.triggerIn(getInput<TriggerIn>(v).value_or(0.f)));

		// TODO: ping input originall has internal lowpass filtering
		if (voice.pingEdge(voice.pingIn(getInput<PingJackIn>(v).value_or(0.f)))) {
			peg.pingEdgeIn();
		}

		setOutput<EofOut>(peg.digio.EOJack.sideload_get() ? TriggerOutputInV : 0.f, v);

		auto MapDACFunc = [](auto val) -> float {
			return float(val) / 4095.f;
		};

		setOutput<EnvOut>(MapDACFunc(peg.dac_vals[0]) * EnvelopeOutFullScaleInV + EnvelopeOutOffsetInV, v);
		setOutput<_5VEnvOut>(MapDACFunc(peg.dac_vals[1]) * 5.f, v);

		// Panel LEDs show the first voice
		if (v != 0)
			return;

		auto PWMToFloatFunc = [](uint16_t pwm_val) -> float {
			return float(pwm_val) / float(4095);
		};

		setLED<EnvOutLight>(std::array<float, 3>{PWMToFloatFunc(peg.pwm_vals[PWM_ENVA_R]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_ENVA_G]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_ENVA_B])});
		setLED<_5VEnvLight>(std::array<float, 3>{PWMToFloatFunc(peg.pwm_vals[PWM_ENVB_R]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_ENVB_G]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_ENVB_B])});
		setLED<CycleButton>(std::array<float, 3>{PWMToFloatFunc(peg.pwm_vals[PWM_CYCLEBUT_R]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_CYCLEBUT_G]),
												 PWMToFloatFunc(peg.pwm_vals[PWM_CYCLEBUT_B])});
		setLED<PingButton>(std::array<float, 3>{PWMToFloatFunc(peg.pwm_vals[PWM_PINGBUT_R]),
												PWMToFloatFunc(peg.pwm_vals[PWM_PINGBUT_G]),
												PWMToFloatFunc(peg.pwm_vals[PWM_PINGBUT_B])});
		setLED<EofLight>(PWMToFloatFunc(peg.pwm_vals[PWM_EOF_LED]));
	}

	void sideloadSystemSettings(unsigned v) {
		auto &peg = voices[v].peg;

		peg.settings.limit_skew = getState<SkewLimitAltParam>() == 1 ? 1 : 0;
		peg.settings.free_running_ping = getState<FreeNRunningPingAltParam>() == 0 ? 1 : 0;
		peg.settings.trigout_is_trig = getState<EofJackTypeAltParam>() == 1 ? 1 : 0;

		static constexpr std::array<TrigOutFunctions, 4> TrigOutOptions{
			TRIGOUT_IS_ENDOFRISE, TRIGOUT_IS_ENDOFFALL, TRIGOUT_IS_HALFRISE, TRIGOUT_IS_TAPCLKOUT};

		peg.settings.trigout_function = TrigOutOptions[getState<EofJackModeAltParam>()];

		// TODO: check if this mapping is correct
		static constexpr std::array<CycleJackBehaviors, 3> CycleJackOptions{
			CYCLE_JACK_RISING_EDGE_TOGGLES, CYCLE_JACK_BOTH_EDGES_TOGGLES_QNT, CYCLE_JACK_BOTH_EDGES_TOGGLES};

		peg.settings.cycle_jack_behavior = CycleJackOptions[getState<CycleJackModeAltParam>()];

		static constexpr std::array<TrigInFunctions, 3> TriggerInOptions{
			TRIGIN_IS_ASYNC, TRIGIN_IS_ASYNC_SUSTAIN, TRIGIN_IS_QNT};

		peg.settings.trigin_function = TriggerInOptions[getState<TrigJackModeAltParam>()];
		peg.settings.shift_value = getState<ShiftAltParam>() * 4095.f;

		peg.set_sync_mode(getState<SyncModeAltParam>() == 0);
	}

private:
	static constexpr float CVInputFullScaleInV = -10.0f;
	static constexpr float TriggerOutputInV = 5.0f;
	static constexpr float EnvelopeOutFullScaleInV = -20.0f;
	static constexpr float EnvelopeOutOffsetInV = 10.0f;

private:
	struct Voice {
		PEG::MiniPEGEnvelopeCalcs env_calcs;
		PEG::PEGBase peg{&env_calcs};

		FlipFlop pingIn{1.f, 2.f};
		FlipFlop cycleIn{1.f, 2.f};
		FlipFlop triggerIn{1.f, 2.f};
		EdgeDetector pingEdge;
	};
	std::array<Voice, MaxChans> voices{};

private:
	float timerPhase = 0;
	float timerPhaseIncrement = 1.0f;

public:
	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType(Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on
};

} // namespace MetaModule
