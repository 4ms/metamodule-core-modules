#pragma once

#include "processors/tools/multireadDelayLine.h"
#include "util/math.hh"
#include "util/math_tables.hh"

template<int maxWindowSize>
class PitchShift {
private:
	MultireadDelayLine<maxWindowSize> pitchDelay;
	float phaccu = 0;
	const int incrementalPitch = 1;
	float sampleRate = 48000;

public:
	PitchShift() = default;

	float update(float input) {
		float output1 = 0;
		float output2 = 0;
		if (incrementalPitch == 0)
			shiftAmount = (long)shiftAmount;
		output1 = pitchDelay.readSample(phaccu * windowSize);
		float adjustedPhase = phaccu + 0.5f;
		if (adjustedPhase >= 1)
			adjustedPhase -= 1.0f;
		output2 = pitchDelay.readSample(adjustedPhase * windowSize);
		float window1 = 0;
		float window2 = 0;

		window1 = sinTable.interp(std::clamp(phaccu * 0.5f, 0.f, 1.f));
		window2 = sinTable.interp(std::clamp(adjustedPhase * 0.5f, 0.f, 1.f));

		float pitchToFreq = 0;
		float pitchShift = std::clamp(shiftAmount / 60.0f, -1.f, 1.f);
		if (pitchShift >= 0)
			pitchToFreq = ((exp5Table.interp(pitchShift) - 1.0f) * -1.0f) / windowSize;
		else {
			pitchToFreq = ((1.0f / exp5Table.interp(-pitchShift) - 1.0f) * -1.0f) / windowSize;
		}

		phaccu += pitchToFreq;
		if (phaccu >= 1)
			phaccu -= 1.0f;
		if (phaccu < 0)
			phaccu += 1.0f;
		pitchDelay.updateSample(input);
		pitchDelay.incrementWriteHead();
		float wet = output1 * window1 + output2 * window2;
		return MathTools::interpolate(input, wet, mix);
	}

	void setSampleRate(float sr) {
		if (sr > 0.f)
			sampleRate = sr;
	}

	float shiftAmount = 1;
	float windowSize = 500;
	float mix = 0;
};
