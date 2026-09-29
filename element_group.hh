#pragma once
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace MetaModule
{

// Element groups: elements that belong to a group are collapsed into a single row
// in the module view's element list, which opens a list of just that group.
//
// Groups are declared in a plugin's plugin-mm.json, per module slug, and applied to
// the registry after the plugin's modules are registered. See docs/element-groups.md
// in the plugin SDK.

// One member of a group, as written in plugin-mm.json: either the element's name as
// it appears on screen, or a typed index -- "elem:N", "param:N", "in:N", "out:N",
// "light:N". Names are matched when the group is first shown, since a module's
// element names aren't final until the module has been created.
struct ElementRef {
	enum class Kind : uint8_t {
		Name,		// match the element's short_name
		ElementIdx, // index into the module's Elements array
		Param,		// paramId
		Input,		// inputId
		Output,		// outputId
		Light,		// firstLightId
	};

	Kind kind = Kind::Name;
	uint16_t idx = 0;
	std::string name; // Kind::Name only

	static ElementRef parse(std::string_view token) {
		struct Prefix {
			std::string_view text;
			Kind kind;
		};
		constexpr Prefix prefixes[]{
			{"elem:", Kind::ElementIdx},
			{"param:", Kind::Param},
			{"in:", Kind::Input},
			{"out:", Kind::Output},
			{"light:", Kind::Light},
		};

		for (auto const &prefix : prefixes) {
			if (!token.starts_with(prefix.text))
				continue;

			auto digits = token.substr(prefix.text.size());
			unsigned value{};
			auto [end, err] = std::from_chars(digits.data(), digits.data() + digits.size(), value);

			// Only a prefix followed by digits and nothing else is a typed index:
			// anything else is an element that happens to be named like one
			if (err == std::errc{} && end == digits.data() + digits.size() && value <= UINT16_MAX)
				return {prefix.kind, (uint16_t)value, {}};

			break;
		}

		return {Kind::Name, 0, std::string(token)};
	}

	// For log messages: an ElementRef as it was written in plugin-mm.json
	std::string describe() const {
		switch (kind) {
			case ElementRef::Kind::Name:
				return name;
			case ElementRef::Kind::ElementIdx:
				return "elem:" + std::to_string(idx);
			case ElementRef::Kind::Param:
				return "param:" + std::to_string(idx);
			case ElementRef::Kind::Input:
				return "in:" + std::to_string(idx);
			case ElementRef::Kind::Output:
				return "out:" + std::to_string(idx);
			case ElementRef::Kind::Light:
				return "light:" + std::to_string(idx);
		}
		return "?";
	}
};

struct ElementGroup {
	std::string name;
	std::vector<ElementRef> members;
};

// A name to show for an element in the module view's element list, instead of its own name
struct ElementName {
	ElementRef element;
	std::string name;
};

} // namespace MetaModule
