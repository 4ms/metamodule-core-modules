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

static constexpr std::array<uint32_t, NumUserFacingInJacks> in_order{0, 1, 2, 3, 4, 5};

static constexpr std::array<std::string_view, NumUserFacingInJacks> InJackNames{
	"In9", "In10", "In11", "In12", "In13", "In14"};

static constexpr std::array<std::string_view, NumUserFacingInJacks> InJackAbbrevs{"9", "10", "11", "12", "13", "14"};

static constexpr std::string_view get_map_injack_name(uint32_t id) {
	if (id < InJackNames.size())
		return InJackNames[id];
	return "?";
}

static constexpr uint32_t get_map_injack_num(uint32_t id) {
	id = std::clamp(id, 0u, NumUserFacingInJacks);
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
	id = std::clamp(id, 0u, NumUserFacingOutJacks);
	return id + PanelDef::NumUserFacingOutJacks;
}

// Jack sensing
constexpr inline unsigned jacksense_pin_order[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

constexpr inline bool jack_is_patched(uint32_t jack_sense_reading, unsigned panel_jack_idx) {
	return (jack_sense_reading >> jacksense_pin_order[panel_jack_idx]) & 1;
}

} // namespace MetaModule::AudioExpander
