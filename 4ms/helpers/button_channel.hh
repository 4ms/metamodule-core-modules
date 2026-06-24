#pragma once
#include <algorithm>
#include <cmath>

// Shared push-button channel logic for the Button Utilities modules
// (Octopush, SoloPush). This is the pure-DSP portion of the original VCV
// plugin.hpp ButtonUtils namespace, with no Rack dependencies.

namespace MetaModule::ButtonUtils
{

enum BehaviorMode { Gate = 0, Toggle = 1, Trigger = 2 };

// Range switch positions: 1V, 5V, 10V
inline constexpr float RangeScales[] = {1.f, 5.f, 10.f};

struct ButtonChannel {
	bool prevButtonState = false;
	bool toggleState = false;
	bool trigState = false;
	float trigTimeRemaining = 0.f;
	float trigLightTimeRemaining = 0.f;
	int prevBehaviorMode = -1;

	struct Output {
		float logicOut = 0.f;
		float voltageOut = 0.f;
		float ledRed = 0.f;
		float ledGreen = 0.f;
		float pushLedBrightness = 0.f;
		bool modeChanged = false;
	};

	Output process(bool buttonPressed, int mode, bool polarity, float amplitude, float scale, float sampleTime) {
		Output out;

		if (mode != prevBehaviorMode) {
			toggleState = false;
			trigState = false;
			trigTimeRemaining = 0.f;
			trigLightTimeRemaining = 0.f;
			prevButtonState = false;
			prevBehaviorMode = mode;
			out.modeChanged = true;
			return out;
		}

		const bool prevPressed = prevButtonState;
		prevButtonState = buttonPressed;
		const bool risingEdge = buttonPressed && !prevPressed;

		switch (mode) {
			case Gate:
				out.logicOut = buttonPressed ? 5.f : 0.f;
				break;
			case Toggle:
				if (risingEdge)
					toggleState = !toggleState;
				out.logicOut = toggleState ? 5.f : 0.f;
				break;
			case Trigger:
				if (risingEdge) {
					trigState = true;
					trigTimeRemaining = 0.005f;
					trigLightTimeRemaining = 0.1f;
				}
				if (trigState) {
					trigTimeRemaining -= sampleTime;
					if (trigTimeRemaining <= 0.f)
						trigState = false;
					else
						out.logicOut = 5.f;
				}
				break;
		}

		out.voltageOut = out.logicOut > 0.f ? (polarity ? (amplitude * 2.f - 1.f) * scale : amplitude * scale) : 0.f;

		// Push button LED (holds bright for 100ms after trigger)
		if (trigLightTimeRemaining > 0.f) {
			trigLightTimeRemaining -= sampleTime;
			out.pushLedBrightness = 1.f;
		} else {
			out.pushLedBrightness = out.logicOut > 0.f ? 1.f : 0.f;
		}

		// Voltage LED colors
		if (polarity) {
			float brightness = std::min(std::abs(out.voltageOut) / scale, 1.f);
			out.ledGreen = (out.voltageOut >= 0.f) ? brightness : 0.f;
			out.ledRed = (out.voltageOut < 0.f) ? brightness : 0.f;
		} else {
			out.ledGreen = std::min(out.voltageOut / scale, 1.f);
			out.ledRed = 0.f;
		}

		return out;
	}
};

} // namespace MetaModule::ButtonUtils
