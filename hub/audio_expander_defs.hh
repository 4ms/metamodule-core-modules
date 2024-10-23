#pragma once
#include "panel_medium_defs.hh"
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace MetaModule::AudioExpander
{
/// Ins

static constexpr uint32_t NumInJacks = 6;

static constexpr std::array<uint32_t, NumInJacks> in_order{0, 1, 2, 3, 4, 5};

static constexpr std::array<std::string_view, NumInJacks> InJackNames{"In9", "In10", "In11", "In12", "In13", "In14"};

static constexpr std::array<std::string_view, NumInJacks> InJackAbbrevs{"9", "10", "11", "12", "13", "14"};

static constexpr std::string_view get_map_injack_name(uint32_t id) {
	if (id < InJackNames.size())
		return InJackNames[id];
	return "?";
}

//// Outs

static constexpr std::array<uint32_t, 8> out_order{0, 4, 6, 7, 5, 3, 1, 2};

static constexpr uint32_t NumOutJacks = 8;
static constexpr uint32_t NumUserFacingOutJacks = NumOutJacks;

static constexpr std::array<std::string_view, NumUserFacingOutJacks> OutJackNames{
	"Out9", "Out10", "Out11", "Out12", "Out13", "Out14", "Out15", "Out16"};

static constexpr std::array<std::string_view, NumUserFacingOutJacks> OutJackAbbrevs{
	"9", "10", "11", "12", "13", "14", "15", "16"};

static constexpr std::string_view get_map_outjack_name(uint32_t id) {
	if (id < OutJackNames.size())
		return OutJackNames[id];
	return "?";
}

//////////

static constexpr uint32_t exp_to_panel_input(uint32_t exp_injack_idx) {
	return exp_injack_idx + PanelDef::NumUserFacingInJacks;
}

static constexpr unsigned panel_to_exp_input(unsigned panel_injack_idx) {
	return panel_injack_idx - PanelDef::NumUserFacingInJacks;
}

static constexpr uint32_t exp_to_panel_output(uint32_t exp_outjack_idx) {
	return exp_outjack_idx + PanelDef::NumUserFacingOutJacks;
}

static constexpr uint32_t panel_to_exp_output(uint32_t panel_outjack_idx) {
	return panel_outjack_idx - PanelDef::NumUserFacingOutJacks;
}

static constexpr bool is_expander_input(unsigned panel_injack_idx) {
	return panel_injack_idx < PanelDef::NumUserFacingInJacks + NumInJacks &&
		   panel_injack_idx >= PanelDef::NumUserFacingInJacks;
}

static constexpr bool is_expander_output(unsigned panel_outjack_idx) {
	return panel_outjack_idx < PanelDef::NumUserFacingOutJacks + NumOutJacks &&
		   panel_outjack_idx >= PanelDef::NumUserFacingOutJacks;
}

// Jack sensing:
// Note: pins 8 and 10 are not connected (that is, bits 24 and 26 of jack_sense are not used)
constexpr unsigned jacksense_pin_order[14] = {/*in:*/ 16, 21, 17, 20, 19, 23, /*out:*/ 18, 22, 31, 25, 30, 27, 29, 28};

constexpr inline std::optional<unsigned> jacksense_output_bit(unsigned panel_outjack_idx) {
	if (is_expander_output(panel_outjack_idx))
		return jacksense_pin_order[panel_to_exp_output(panel_outjack_idx) + NumInJacks];
	else
		return {};
}

constexpr inline std::optional<unsigned> jacksense_input_bit(unsigned panel_injack_idx) {
	if (is_expander_output(panel_injack_idx))
		return jacksense_pin_order[panel_to_exp_input(panel_injack_idx)];
	else
		return {};
}

// Given expander jack ID (0..5 = Expander Inputs, 6..13 = Expander Outputs)
// Not bounds-checked (used in audio loop)
constexpr inline bool jack_is_patched(uint32_t jack_sense_reading, unsigned exp_panel_jack_idx) {
	return (jack_sense_reading >> jacksense_pin_order[exp_panel_jack_idx]) & 1;
}

} // namespace MetaModule::AudioExpander
