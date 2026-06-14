#pragma once
#include "../easiglib/dsp.hh"
#include "../parameters.hh"

namespace EnOsc
{

enum SpiAdcInput { CV_PITCH, CV_ROOT, NUM_SPI_ADC_CHANNELS };

// Model of the pitch/root CV inputs, one value per poly channel each.
class SpiAdc : easiglib::Nocopy {
	easiglib::u0_16 values[NUM_SPI_ADC_CHANNELS][kMaxPolyChans];

public:
	SpiAdc() = default;

	void switch_channel() {
	}

	void set(int input, int poly_chan, easiglib::u0_16 v) {
		static_assert(NUM_SPI_ADC_CHANNELS == 2);
		static_assert(kMaxPolyChans == 4);
		values[input & 1][poly_chan & 3] = v; //cheap bounds-checking
	}

	easiglib::u0_16 get(int input, int poly_chan) {
		static_assert(NUM_SPI_ADC_CHANNELS == 2);
		return values[input & 1][poly_chan & 3]; //cheap bounds-checking
	}
};

} // namespace EnOsc
