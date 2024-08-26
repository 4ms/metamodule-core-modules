#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "alpaca/alpaca.h"
#include "ryml.hpp"
#include "ryml_init.hh"

namespace MetaModule::ModuleState
{
// Patch file yaml -> decode -> raw bytes for module
static std::vector<uint8_t> decode(std::string_view base64_string) {
	RymlInit::init_once();

	auto needed_size = c4::base64_decode({base64_string.data(), base64_string.size()}, {});
	std::vector<uint8_t> decoded_data(needed_size);
	c4::base64_decode({base64_string.data(), base64_string.size()}, {decoded_data.data(), decoded_data.size()});

	return decoded_data;
}

// Raw bytes from module -> encode -> patch file yaml
static std::string encode(std::span<const uint8_t> raw_data) {
	RymlInit::init_once();

	// Encode bytes into base64
	auto needed_size = c4::base64_encode({}, {raw_data.data(), raw_data.size()});
	std::string encoded_data(needed_size, '\0');
	c4::base64_encode({encoded_data.data(), encoded_data.size()}, {raw_data.data(), raw_data.size()});
	return encoded_data;
}
} // namespace MetaModule::ModuleState
