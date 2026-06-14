#pragma once

#include <cstdint>

// Where does EnOsc's CPU actually go on hardware?
//
// Build with ENOSC_PROFILE defined (uncomment the #define at the top of
// EnOscCore.cc) on the Cortex-A7 target. EnOscCore then prints a line every
// ~2 seconds like:
//
//   EnOsc: total=13120455 ticks/2s | poll 4% events 0% led 0% | audio 91%
//
// Percentages are relative to the module's own total. The absolute total
// (generic-timer ticks per 2s window) is comparable across builds: lower is
// faster. The counters can also be inspected in a debugger at
// EnOsc::Prof::buckets.
//
// With ENOSC_PROFILE undefined this header compiles to nothing.

namespace EnOsc::Prof
{

#if defined(ENOSC_PROFILE) && defined(CORE_CA7)
// Generic timer (CNTPCT, ~12-24 MHz) — same source the firmware's
// mdrivlib::CycleCounter uses on the A7; coarse but plenty for these windows.
inline uint32_t ticks() {
	uint32_t lo, hi;
	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"(lo), "=r"(hi));
	return lo;
}
#else
inline uint32_t ticks() {
	return 0;
}
#endif

struct Buckets {
	uint32_t control_poll; // Control::Poll + SPI ADC conditioning (6kHz)
	uint32_t ui_events;	   // EventHandler::Process (20Hz)
	uint32_t led_update;   // LED manager (60Hz)
	uint32_t audio_block;  // PolypticOscillator::Process (block rate)
	uint32_t total;		   // whole EnOscCore::update()
};
inline Buckets buckets{};

struct Scope {
	uint32_t &acc;
	uint32_t t0;
	explicit Scope(uint32_t &a)
		: acc(a)
		, t0(ticks()) {
	}
	~Scope() {
		acc += ticks() - t0;
	}
};

} // namespace EnOsc::Prof

// no-op (zero ticks) unless ENOSC_PROFILE && CORE_CA7
#define ENOSC_PROF_SCOPE(name) EnOsc::Prof::Scope prof_scope_##name(EnOsc::Prof::buckets.name)
