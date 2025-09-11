#pragma once

#include <cstdio>
class ExpDecay {
public:
	ExpDecay() = default;

	void set_samplerate(float sr) {
		if (sr != sampleRate) {
			float decayms = decay_ms();
			float attackms = attack_ms();

			sampleRate = sr;

			set_decay_ms(decayms);
			set_attack_ms(attackms);
		}
	}

	float update(float input) {
		float lastSample = currentSample;
		currentSample = input;

		if (currentSample > lastSample)
			direction = true;
		else if (currentSample < lastSample)
			direction = false;

		if (direction)
			slewedOut += (input - slewedOut) * attackTime;
		else
			slewedOut += (input - slewedOut) * decayTime;

		return slewedOut;
	}

	float attack_ms() const {
		return 1000.f / (attackTime * sampleRate);
	}

	float decay_ms() const {
		return 1000.f / (decayTime * sampleRate);
	}

	void set_decay_ms(float decay_ms) {
		decayTime = 1000.0f / decay_ms / sampleRate;
	}

	void set_attack_ms(float attack_ms) {
		attackTime = 1000.0f / attack_ms / sampleRate;
	}

private:
	float sampleRate = 48000;
	float slewedOut = 0;
	float currentSample = 0;
	bool direction = false;

	float decayTime = 0.0021f;
	float attackTime = 0.0021f;
};
