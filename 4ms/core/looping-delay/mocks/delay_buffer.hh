#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "conf.hh"
#include "elements.hh"

namespace LDKit
{

struct DelayBuffer {
	static constexpr uint32_t size = Brain::MemorySizeBytes / sizeof(int16_t);
	using array = std::array<int16_t, size>;
	using span = std::span<int16_t, size>;

	DelayBuffer()
		: sp_{buf_.data(), buf_.size()} {
	}

	DelayBuffer::span &get() {
		return sp_;
	}

	void clear() {
		std::fill(buf_.begin(), buf_.end(), 0);
	}

private:
	DelayBuffer::array buf_;
	DelayBuffer::span sp_;
};

} // namespace LDKit
