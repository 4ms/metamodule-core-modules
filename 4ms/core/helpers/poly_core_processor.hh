#pragma once
#include "CoreModules/CoreProcessor.hh"
#include <algorithm>
#include <array>
#include <cstdint>

// Tell the vectorizer there are no loop-carried dependencies, so it doesn't
// emit runtime overlap checks with scalar fallback loops (GCC only exploits
// __restrict on function parameters, not on local pointers).
#ifndef MM_NO_LOOP_DEPS
#if defined(__clang__)
#define MM_NO_LOOP_DEPS _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define MM_NO_LOOP_DEPS _Pragma("GCC ivdep")
#else
#define MM_NO_LOOP_DEPS
#endif
#endif

namespace MetaModule
{

// Storage for one polyphonic jack, matching the patch player's PolyPortBuffer
// contract: the player reads/writes values[] and chans (0 = unpatched,
// 1 = mono source, 2+ = poly cable).
struct PolyJack {
	static constexpr unsigned Max = CoreProcessor::MaxPolyChannels;

	alignas(16) std::array<float, Max> values{};
	uint8_t chans = 0;

	CoreProcessor::PolyPortBuffer buffer() {
		return {values.data(), &chans};
	}

	bool is_patched() const {
		return chans > 0;
	}

	void mark_unpatched() {
		chans = 0;
		values = {};
	}

	// 0 -> 1 channel of 0V; a poly source will then raise the count itself
	void mark_patched() {
		if (chans == 0) {
			chans = 1;
			values = {};
		}
	}

	// Channel ch, with the highest channel feeding all upper channels.
	// Returns 0 if unpatched.
	float or_last(unsigned ch) const {
		return chans ? values[std::min<unsigned>(ch, chans - 1u)] : 0.f;
	}

	// Expand to a fixed-size per-voice array (highest channel feeds upward),
	// optionally scaled. The fixed-size output keeps downstream loops
	// vectorizable.
	void expand(std::array<float, Max> &out, float scale = 1.f) const {
		if (chans == 0) {
			out = {};
		} else {
			for (unsigned v = 0; v < Max; v++)
				out[v] = values[std::min<unsigned>(v, chans - 1u)] * scale;
		}
	}
};

// Base class for simple polyphonic modules with jacks indexed by their
// (contiguous, 0-based) jack ids. Implements the CoreProcessor poly-buffer
// plumbing and the mono set_input/get_output fallbacks; derived modules
// implement update()/set_param()/etc. using ins[] and outs[].
template<unsigned NumIns, unsigned NumOuts>
class PolyCoreProcessor : public CoreProcessorPoly {
public:
	static constexpr unsigned MaxVoices = MaxPolyChannels;

	PolyCoreProcessor() {
		for (auto &out : outs)
			out.chans = 1;
	}

	PolyPortBuffer get_poly_input_buffer(int input_id) override {
		return (size_t)input_id < ins.size() ? ins[input_id].buffer() : PolyPortBuffer{};
	}

	PolyPortBuffer get_poly_output_buffer(int output_id) override {
		return (size_t)output_id < outs.size() ? outs[output_id].buffer() : PolyPortBuffer{};
	}

	// Mono sources write channel 0; poly sources write the buffers directly
	void set_input(int input_id, float val) override {
		if ((size_t)input_id < ins.size())
			ins[input_id].values[0] = val;
	}

	float get_output(int output_id) const override {
		return (size_t)output_id < outs.size() ? outs[output_id].values[0] : 0.f;
	}

	void mark_all_inputs_unpatched() override {
		for (auto &jack : ins)
			jack.mark_unpatched();
	}

	void mark_input_unpatched(int input_id) override {
		if ((size_t)input_id < ins.size())
			ins[input_id].mark_unpatched();
	}

	void mark_input_patched(int input_id) override {
		if ((size_t)input_id < ins.size())
			ins[input_id].mark_patched();
	}

protected:
	// Voice count set by a leader input (1 voice if unpatched or mono)
	unsigned num_voices(unsigned input_id) const {
		return std::clamp<unsigned>(ins[input_id].chans, 1, MaxVoices);
	}

	std::array<PolyJack, NumIns> ins{};
	std::array<PolyJack, NumOuts> outs{};
};

} // namespace MetaModule
