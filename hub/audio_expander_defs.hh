#pragma once
#include "panel_medium_defs.hh"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

namespace MetaModule::AudioExpander
{
/// Ins

static constexpr uint32_t NumInJacks = 6;
static constexpr uint32_t NumUserFacingInJacks = NumInJacks;

static constexpr std::array<uint32_t, NumUserFacingInJacks> in_order{0, 1, 2, 3, 4, 5}; // FIXME

static constexpr std::array<std::string_view, NumUserFacingInJacks> InJackNames{
	"In9", "In10", "In11", "In12", "In13", "In14"};

static constexpr std::array<std::string_view, NumUserFacingInJacks> InJackAbbrevs{"9", "10", "11", "12", "13", "14"};

static constexpr std::string_view get_map_injack_name(uint32_t id) {
	if (id < InJackNames.size())
		return InJackNames[id];
	return "?";
}

// 0..5 => 8..13
static constexpr uint32_t get_map_injack_num(uint32_t id) {
	id = std::clamp<uint32_t>(id, 0u, NumInJacks - 1);
	return id + PanelDef::NumUserFacingInJacks;
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

static constexpr uint32_t get_map_outjack_num(uint32_t id) {
	id = std::clamp<uint32_t>(id, 0u, NumUserFacingOutJacks);
	return id + PanelDef::NumUserFacingOutJacks;
}

// Jack sensing: pins 8 and 10 are not connected (that is, bits 24 and 26 of jack_sense are not used)
constexpr inline unsigned jacksense_pin_order[14] = {16, 21, 17, 20, 19, 23, /*out:*/ 18, 22, 31, 25, 30, 27, 29, 28};
// 											 /*ins:*/ 0,  5,  1,  4,  3,  7, /*outs:*/ 2,  6, 15,  9, 14, 11, 13, 12};

// Given expander jack ID (0..5 = Expander Inputs, 6..13 = Expander Outputs)
constexpr inline bool jack_is_patched(uint32_t jack_sense_reading, unsigned exp_panel_jack_idx) {
	return (jack_sense_reading >> jacksense_pin_order[exp_panel_jack_idx]) & 1;
}

constexpr inline bool input_jack_is_patched(uint32_t jack_sense_reading, unsigned exp_panel_jack_idx) {
	//No range checking: asset 0 <= exp_panel_jack_idx < NumInJacks
	return (jack_sense_reading >> jacksense_pin_order[exp_panel_jack_idx]) & 1;
}

constexpr inline bool output_jack_is_patched(uint32_t jack_sense_reading, unsigned exp_panel_jack_idx) {
	//No range checking: asset 0 <= exp_panel_jack_idx < NumOutJacks
	return (jack_sense_reading >> jacksense_pin_order[exp_panel_jack_idx + NumInJacks]) & 1;
}

} // namespace MetaModule::AudioExpander
