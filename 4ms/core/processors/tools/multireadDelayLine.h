#pragma once
#include "util/interp_array.hh"
#include "util/math.hh"

template<int maxSamples>
class MultireadDelayLine {
public:
	MultireadDelayLine() {
		for (int i = 0; i < maxSamples; i++) {
			delayBuffer[i] = 0;
		}
	}

	void set_samplerate(float sr) {
	}

	void updateSample(float input) {
		if (writeIndex < maxSamples)
			delayBuffer[writeIndex] = input;
	}

	void incrementWriteHead() {
		writeIndex++;
		if (writeIndex >= maxSamples)
			writeIndex = 0;
	}

	float readSample(float delaySamples) {
		float readIndex = writeIndex - delaySamples;
		while (readIndex < 0)
			readIndex += maxSamples;

		while (readIndex >= maxSamples)
			readIndex -= maxSamples;

		return delayBuffer.interp_by_index(readIndex);
	}

private:
	InterpArray<float, maxSamples> delayBuffer;
	unsigned long writeIndex = 0;
};
