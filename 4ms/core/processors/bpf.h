#pragma once

#include "util/math.hh"
#include "util/parameter.hh"
#include <algorithm>
#include <cmath>

using namespace MathTools;

class BandPassFilter {
public:
	Parameter<float> cutoff;
	Parameter<float> q;
	Parameter<float> sampleRate;

	float update(float input) {
		float output = 0;
		if (sampleRate.isChanged()) {
			auto tempSamplerate = constrain(sampleRate.getValue(), 1.0f, 192000.0f);
			fConst0 = (3.14159274f / min<float>(192000.0f, max<float>(1.0f, float(tempSamplerate))));
		}
		if (q.isChanged() || cutoff.isChanged()) {
			calcFilterVariables();
		}

		fRec0[0] = (float(input) - (fSlow5 * ((fSlow6 * fRec0[2]) + (fSlow7 * fRec0[1]))));
		output = float(((fSlow4 * fRec0[0]) + (fSlow8 * fRec0[2])));
		fRec0[2] = fRec0[1];
		fRec0[1] = fRec0[0];
		return output; //constrain(output, -10.0f, 10.0f);
	}

	BandPassFilter() {
		for (int i = 0; i < 3; i++) {
			fRec0[i] = 0.0f;
		}
		cutoff.setValue(262);
		q.setValue(1);
		sampleRate.setValue(48000);
	}

private:
	float fRec0[3];
	float fSlow0, fSlow1, fSlow2, fSlow3, fSlow4, fSlow5, fSlow6, fSlow7, fSlow8;
	float fConst0 = 1;

	void calcFilterVariables() {
		fSlow0 = std::tan(fConst0 * std::max(float(cutoff.getValue()), 0.00001f));
		fSlow1 = 1.0f / std::max(float(q.getValue()), 0.0001f);
		fSlow2 = 1.0f / fSlow0;
		fSlow3 = ((fSlow1 + fSlow2) / fSlow0) + 1.0f;
		fSlow4 = 1.0f / (fSlow0 * fSlow3);
		fSlow5 = 1.0f / fSlow3;
		fSlow6 = ((fSlow2 - fSlow1) / fSlow0) + 1.0f;
		fSlow7 = 2.0f * (1.0f - (1.0f / (fSlow0 * fSlow0)));
		fSlow8 = 0.0f - fSlow4;
	}
};
