#pragma once
#include "gcem/include/gcem.hpp"
#include "util/lookup_table.hh"

struct Log10_1_10InputRange {
	static constexpr float min = 1.f;
	static constexpr float max = 10.f;
};

static constinit auto Log10_1_10 =
	LookupTable<64>::generate<Log10_1_10InputRange>([](float input) { return gcem::log(input) / gcem::log(10.0); });
