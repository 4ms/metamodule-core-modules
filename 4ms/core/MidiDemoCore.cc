#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/register_module.hh"
#include "midi/midi_in.hh"
#include "midi/midi_out.hh"
#include <cmath>

namespace MetaModule
{

struct MidiDemoInfo {
	static constexpr std::string_view slug{"MidiDemo"};
	static constexpr uint32_t width_hp = 4;

	static constexpr std::array<Element, 2> Elements{{
		JackOutput{},
		JackInput{},
	}};

	enum class Elem {
		CC10toCV,
		CVtoCC74,
	};
};

class MidiDemoCore : public SmartCoreProcessor<MidiDemoInfo> {
	using Info = MidiDemoInfo;
	using ThisCore = MidiDemoCore;

public:
	MidiDemoCore() = default;

	void update(void) override {
		// Receiving CC10 will set the output voltage
		if (auto msg = midi_in.pop_message()) {
			// Print all MIDI messages to console
			printf("MIDI RX: 0x%08x\n", msg->raw());
			if (msg->is_command<MidiCommand::ControlChange>()) {
				if (msg->ccnum() == 10) {
					setOutput<MidiDemoInfo::Elem::CC10toCV>(msg->ccval() / 12.7f); //0-127 => 0-10V
				}
			}
		}

		// Voltage changing on the CV input jack will send a CC74 message
		int cv = std::round(getInput<MidiDemoInfo::Elem::CVtoCC74>().value_or(0) * 12.7f);
		if (last_cv_val != cv) {
			last_cv_val = cv;

			if (!midi_out.is_queue_full()) {
				MidiMessage msg(0xB0, 74, std::clamp(cv, 0, 127));
				midi_out.push_message(msg);
			}
		}
	}

	void set_samplerate(const float sr) override {
	}

	static inline bool s_registered = register_module<MidiDemoCore, MidiDemoInfo>("4msCompany");

private:
	MidiOutput midi_out;
	MidiInput midi_in;

	int last_cv_val = 0;
};

} // namespace MetaModule
