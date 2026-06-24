#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/moduleFactory.hh"
#include "info/Keyboard_info.hh"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <utility>

namespace MetaModule
{

// Polyphonic note keyboard: 12 momentary keys feed a voice allocator that
// drives 8 mono CV/Gate pairs plus polyphonic CV/Gate cables. Keys behave as
// Gate / Trig / Latch; voices are assigned by First Available / Round Robin /
// Random and stolen by the selected Note Priority. Each key's RGB LED shows
// blue when its note holds a voice, red when it was dropped (no free voice).
//
// Note: MetaModule poly cables carry up to MaxPolyChannels (4) channels, so the
// CV Poly / Gate Poly outputs expose only the first 4 voices; the 8 mono CV /
// Gate jacks always cover all voices.
class KeyboardCore : public SmartCoreProcessorPoly<KeyboardInfo> {
	using Info = KeyboardInfo;
	using ThisCore = KeyboardCore;
	using enum Info::Elem;

	static constexpr int NumKeys = 12;
	static constexpr int MaxVoices = 8;

	enum KeyBehavior { Gate = 0, Trig = 1, Latch = 2 };
	enum NotePriority { OldestNote = 0, NewestNote = 1, LowestNote = 2, HighestNote = 3, IgnoreNewNotes = 4 };
	enum VoiceAlloc { FirstAvailable = 0, RoundRobin = 1, Random = 2 };

public:
	KeyboardCore() = default;

	void update() override {
		if (bypassed) {
			handle_bypass();
			return;
		}

		const int mode = (int)getState<BehaviorSwitch>();
		const int numVoices = (int)getState<NumVoicesSwitch>() + 1;
		const int notePriority = (int)getState<NotePrioritySwitch>();
		const int voiceAlloc = (int)getState<VoiceAllocSwitch>();
		const int octaveVal = std::min(4, (int)getState<OctaveSwitch>());
		const float octaveOffset = (float)(octaveVal - 2);
		currentNumVoices = numVoices;

		// On behavior change, clear all per-key state, voices and LEDs.
		if (mode != prevBehaviorMode) {
			for (int i = 0; i < NumKeys; i++) {
				toggleState[i] = false;
				noteTrigRemaining[i] = 0.f;
				trigLightRemaining[i] = 0.f;
				trigWasVoiced[i] = false;
				prevButtonState[i] = false;
				noteActive[i] = false;
				notePrevActive[i] = false;
			}
			for (int v = 0; v < MaxVoices; v++)
				voices[v] = Voice();
			prevBehaviorMode = mode;
			unroll<NumKeys>([&]<unsigned i>() { clearKeyLED<i>(); });
			return;
		}

		// Free voices that fall outside a reduced voice count.
		if (numVoices < prevNumVoices) {
			for (int v = numVoices; v < prevNumVoices; v++) {
				voices[v].note = -1;
				voices[v].gateHigh = false;
			}
		}
		prevNumVoices = numVoices;

		// 1) Read each key into noteActive[] according to the behavior mode.
		unroll<NumKeys>([&]<unsigned i>() { readKey<i>(mode); });

		// 2) Edge-detect note on/off and run the voice allocator.
		for (int i = 0; i < NumKeys; i++) {
			const bool noteOn = noteActive[i] && !notePrevActive[i];
			const bool noteOff = !noteActive[i] && notePrevActive[i];
			notePrevActive[i] = noteActive[i];
			const float cv = octaveOffset + i / (float)NumKeys;
			if (noteOn)
				handleNoteOn(i, cv, numVoices, notePriority, voiceAlloc);
			if (noteOff)
				handleNoteOff(i, numVoices);
		}

		// 3) Update each key's RGB LED.
		unroll<NumKeys>([&]<unsigned i>() { writeKeyLED<i>(mode, numVoices); });

		// 4) Mono CV/Gate outputs + activity LEDs, one per voice slot.
		unroll<MaxVoices>([&]<unsigned v>() { writeVoiceOut<v>(numVoices); });

		// 5) Polyphonic CV/Gate cables (capped at MaxPolyChannels).
		const int polyChans = std::min(numVoices, (int)MaxPolyChannels);
		setChannels<CvPoly>(polyChans);
		setChannels<GatePoly>(polyChans);
		for (int v = 0; v < polyChans; v++) {
			setOutput<CvPoly>(voices[v].cv, v);
			setOutput<GatePoly>(voices[v].gateHigh ? 5.f : 0.f, v);
		}

		// 6) Gate Sum (any key held) and its trigger.
		bool anyButtonPressed = false;
		for (int i = 0; i < NumKeys; i++)
			if (prevButtonState[i]) {
				anyButtonPressed = true;
				break;
			}
		setOutput<GateSum>(anyButtonPressed ? 5.f : 0.f);

		if (anyButtonPressed && !prevGateSumHigh)
			trigSumRemaining = 0.005f;
		prevGateSumHigh = anyButtonPressed;

		if (trigSumRemaining > 0.f) {
			trigSumRemaining -= timeStepInS;
			setOutput<TrigSum>(5.f);
		} else {
			setOutput<TrigSum>(0.f);
		}
	}

	size_t get_display_text(int display_id, std::span<char> text) override {
		const int key = display_id - (int)display_idx<VoiceLabel1>;
		if (key < 0 || key >= NumKeys)
			return 0;
		for (int v = 0; v < currentNumVoices; v++) {
			if (voices[v].note == key) {
				char buf[8];
				int n = std::snprintf(buf, sizeof(buf), "%d", v + 1);
				if (n < 0)
					return 0;
				return copy_text(std::string_view(buf, (size_t)n), text);
			}
		}
		return 0;
	}

	void set_samplerate(float sr) override {
		timeStepInS = 1.f / sr;
	}

	// Boilerplate to auto-register in ModuleFactory
	// clang-format off
	static std::unique_ptr<CoreProcessor> create() { return std::make_unique<ThisCore>(); }
	static inline bool s_registered = ModuleFactory::registerModuleType("4msCompany", Info::slug, create, ModuleInfoView::makeView<Info>(), Info::png_filename);
	// clang-format on

private:
	// ── Compile-time unroll over 0..N-1, calling f.template operator()<i>() ──
	template<unsigned N, typename F>
	void unroll(F &&f) {
		[&]<unsigned... I>(std::integer_sequence<unsigned, I...>) {
			(f.template operator()<I>(), ...);
		}(std::make_integer_sequence<unsigned, N>{});
	}

	template<unsigned i>
	static constexpr Info::Elem keyEl() {
		return static_cast<Info::Elem>(static_cast<int>(Key1) + i);
	}

	template<unsigned i>
	void readKey(int mode) {
		const bool buttonPressed = getState<keyEl<i>()>() == MomentaryButton::State_t::PRESSED;
		const bool risingEdge = buttonPressed && !prevButtonState[i];
		prevButtonState[i] = buttonPressed;

		switch (mode) {
			case Gate:
				noteActive[i] = buttonPressed;
				break;
			case Trig:
				if (risingEdge) {
					noteTrigRemaining[i] = 0.005f;
					trigLightRemaining[i] = 0.1f;
					trigWasVoiced[i] = false;
				}
				if (noteTrigRemaining[i] > 0.f)
					noteTrigRemaining[i] -= timeStepInS;
				if (trigLightRemaining[i] > 0.f)
					trigLightRemaining[i] -= timeStepInS;
				noteActive[i] = (noteTrigRemaining[i] > 0.f);
				break;
			case Latch:
				if (risingEdge)
					toggleState[i] = !toggleState[i];
				noteActive[i] = toggleState[i];
				break;
		}
	}

	template<unsigned i>
	void writeKeyLED(int mode, int numVoices) {
		bool hasVoice = false;
		for (int v = 0; v < numVoices; v++)
			if (voices[v].note == (int)i) {
				hasVoice = true;
				break;
			}

		float r = 0.f, b = 0.f;
		switch (mode) {
			case Gate:
			case Latch:
				if (noteActive[i]) {
					if (hasVoice)
						b = 1.f;
					else
						r = 1.f;
				}
				break;
			case Trig:
				if (trigLightRemaining[i] > 0.f) {
					if (hasVoice)
						trigWasVoiced[i] = true;
					if (trigWasVoiced[i])
						b = 1.f;
					else
						r = 1.f;
				}
				break;
		}
		setLED<keyEl<i>()>(std::array<float, 3>{r, 0.f, b});
	}

	template<unsigned i>
	void clearKeyLED() {
		setLED<keyEl<i>()>(std::array<float, 3>{0.f, 0.f, 0.f});
	}

	template<unsigned v>
	void writeVoiceOut(int numVoices) {
		constexpr auto cvEl = static_cast<Info::Elem>(static_cast<int>(CvOut1) + v);
		constexpr auto gateEl = static_cast<Info::Elem>(static_cast<int>(GateOut1) + v);
		constexpr auto lightEl = static_cast<Info::Elem>(static_cast<int>(CvLight1) + v);

		if ((int)v < numVoices) {
			const bool gh = voices[v].gateHigh;
			setOutput<cvEl>(voices[v].cv);
			setOutput<gateEl>(gh ? 5.f : 0.f);
			setLED<lightEl>(gh ? 1.f : 0.f);
		} else {
			setOutput<cvEl>(0.f);
			setOutput<gateEl>(0.f);
			setLED<lightEl>(0.f);
		}
	}

	// ── Voice state ────────────────────────────────────────────────────
	struct Voice {
		int note = -1; // 0-11, -1 = free
		bool gateHigh = false;
		float cv = 0.f;
		int timestamp = 0;
	};

	int findFreeVoice(int numVoices, int voiceAlloc) {
		std::array<int, MaxVoices> freeVoices;
		int count = 0;
		for (int v = 0; v < numVoices; v++)
			if (voices[v].note < 0)
				freeVoices[count++] = v;
		if (count == 0)
			return -1;

		switch (voiceAlloc) {
			case FirstAvailable:
				return freeVoices[0];
			case RoundRobin: {
				for (int i = 0; i < numVoices; i++) {
					int v = (rrIndex + i) % numVoices;
					if (voices[v].note < 0) {
						rrIndex = (v + 1) % numVoices;
						return v;
					}
				}
				break;
			}
			case Random:
				return freeVoices[std::rand() % count];
		}
		return -1;
	}

	int findStealVoice(int numVoices, int notePriority) {
		int steal = 0;
		switch (notePriority) {
			case OldestNote: {
				int minTs = voices[0].timestamp;
				for (int v = 1; v < numVoices; v++)
					if (voices[v].timestamp < minTs) {
						minTs = voices[v].timestamp;
						steal = v;
					}
				break;
			}
			case NewestNote: {
				int maxTs = voices[0].timestamp;
				for (int v = 1; v < numVoices; v++)
					if (voices[v].timestamp > maxTs) {
						maxTs = voices[v].timestamp;
						steal = v;
					}
				break;
			}
			case LowestNote: {
				int minNote = voices[0].note;
				for (int v = 1; v < numVoices; v++)
					if (voices[v].note >= 0 && voices[v].note < minNote) {
						minNote = voices[v].note;
						steal = v;
					}
				break;
			}
			case HighestNote: {
				int maxNote = voices[0].note;
				for (int v = 1; v < numVoices; v++)
					if (voices[v].note > maxNote) {
						maxNote = voices[v].note;
						steal = v;
					}
				break;
			}
			default:
				break;
		}
		return steal;
	}

	void handleNoteOn(int note, float cv, int numVoices, int notePriority, int voiceAlloc) {
		for (int v = 0; v < numVoices; v++) {
			if (voices[v].note == note) {
				voices[v].gateHigh = true;
				voices[v].timestamp = globalTimestamp++;
				return;
			}
		}
		int v = findFreeVoice(numVoices, voiceAlloc);
		if (v < 0) {
			if (notePriority == IgnoreNewNotes)
				return;
			v = findStealVoice(numVoices, notePriority);
		}
		if (v < 0)
			return;
		voices[v].note = note;
		voices[v].gateHigh = true;
		voices[v].cv = cv;
		voices[v].timestamp = globalTimestamp++;
	}

	void handleNoteOff(int note, int numVoices) {
		for (int v = 0; v < numVoices; v++) {
			if (voices[v].note == note) {
				voices[v].note = -1;
				voices[v].gateHigh = false;
				return;
			}
		}
	}

	// ── State ──────────────────────────────────────────────────────────
	std::array<Voice, MaxVoices> voices{};
	int rrIndex = 0;
	uint64_t globalTimestamp = 0;

	std::array<bool, NumKeys> prevButtonState{};
	std::array<bool, NumKeys> toggleState{};
	std::array<float, NumKeys> noteTrigRemaining{};
	std::array<float, NumKeys> trigLightRemaining{};
	std::array<bool, NumKeys> trigWasVoiced{};
	std::array<bool, NumKeys> noteActive{};
	std::array<bool, NumKeys> notePrevActive{};

	bool prevGateSumHigh = false;
	float trigSumRemaining = 0.f;

	int prevBehaviorMode = -1;
	int prevNumVoices = -1;
	int currentNumVoices = 1;

	float timeStepInS = 1.f / 48000.f;
};

} // namespace MetaModule
