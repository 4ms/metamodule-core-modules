#pragma once

#include "Tables.h"

class Channel {
public:
	Channel() = default;

	void input(float input) {
		inputValue = input;
	}

	void pot(float pot) {
		potValue = pot;
	}

	void control(float control) {
		controlValue = control;
	}

	void mute(bool isMuted) {
		if (isMuted == true) {
			muteValue = 0.f;
		} else {
			muteValue = 1.f;
		}
	}

	float output() {
		outputValue = inputValue * VoltageToGainTable.lookup(potValue * controlValue * muteValue);

		return outputValue;
	}

	float getLEDbrightness() {
		return VoltageToGainTable.lookup(potValue * controlValue * muteValue);
	}

private:
	float potValue{};
	float inputValue{};
	float outputValue{};
	float controlValue{};
	float muteValue{};
};
