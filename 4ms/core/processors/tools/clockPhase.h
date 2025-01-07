#pragma once

#include "schmittTrigger.h"
#include <algorithm>
#include <cstdint>

class ClockPhase {
public:
	void updateClock(float val) {
		auto lastClock = currentClock.output();
		currentClock.update(val);

		// Rising edge:
		if (currentClock.output() && !lastClock) {
			ratio = queueMultiply / queueDivide;
			if (sinceClock > 1)
				duration = sinceClock;
			sinceClock = 0;
			wholeCount++;
		}
	}

	void updateReset(float val) {
		auto lastReset = currentReset.output();
		currentReset.update(val);
		if (currentReset.output() > lastReset)
			wholeCount = 0;
	}

	void update() {
		if (sinceClock < duration) {
			phase = (wholeCount + (float)sinceClock / duration) * ratio;
		}
		sinceClock++; //rolls over every 3 million years or so @96kHz
	}

	float getPhase() {
		return (phase);
	}

	float getWrappedPhase() {
		return (phase - (long)phase);
	}

	long getCount() {
		return wholeCount;
	}

	void setMultiply(int val) {
		queueMultiply = static_cast<float>(val);
	}

	void setDivide(int val) {
		queueDivide = std::max(static_cast<float>(val), 1.0f);
	}

private:
	SchmittTrigger currentClock{0.5f, 1.5f};

	SchmittTrigger currentReset{0.5f, 1.5f};

	unsigned wholeCount = 0;
	int64_t sinceClock = 0;
	float phase = 0;
	float duration = 1000.f;
	float ratio = 1;
	float queueDivide = 1;
	float queueMultiply = 1;
};
