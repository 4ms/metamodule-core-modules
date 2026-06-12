#pragma once
// Test stub: shadows lib/patch-serial/base64.hh so module tests don't need
// to compile rapidyaml. save_state/load_state are not exercised in these tests.
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MetaModule
{
struct Base64 {
	static std::vector<uint8_t> decode(std::string_view) {
		return {};
	}

	static std::string encode(std::span<const uint8_t>) {
		return {};
	}
};
} // namespace MetaModule
