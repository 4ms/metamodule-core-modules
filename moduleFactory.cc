#include "CoreModules/moduleFactory.hh"
#include "util/string_compare.hh"
#include <list>
#include <map>

#if defined(TESTPROJECT)
#define pr_dbg(...)
#define pr_err(...)
#elif defined(VCVRACK)
#define pr_dbg printf
#define pr_err printf
#else
#include "console/pr_dbg.hh"
#endif

namespace MetaModule
{

namespace
{
//Lazily loaded maps, needed to avoid static-initialization ordering issues with CoreModules self-registering

struct ModuleRegistry {
	ModuleFactory::CreateModuleFunc creation_func;
	ModuleInfoView info;
	std::string faceplate;
	std::string display_name;
};

struct BrandRegistry {
	std::string brand_name; //brand slug
	std::string display_name;
	std::vector<std::string> aliases;

	std::map<std::string, ModuleRegistry> modules; //module slug => registry

	BrandRegistry(std::string_view brand)
		: brand_name{brand} {
	}
};

auto &registry() {
	static std::list<BrandRegistry> _registry{};
	return _registry;
}

ModuleInfoView nullinfo{};

} // namespace

// returns a list iterator
static auto brand_registry(std::string_view brand) {
	// First, try to match on brand_name:
	auto found = std::ranges::find(registry(), brand, &BrandRegistry::brand_name);

	// If not found, match case-insensitive
	if (found == registry().end()) {
		found = std::ranges::find_if(
			registry(), [&brand](std::string_view a) { return equal_ci(a, brand); }, &BrandRegistry::brand_name);
	}

	// If still not found, match on alias
	if (found == registry().end()) {
		found = std::ranges::find_if(registry(), [=](BrandRegistry const &reg) {
			return std::ranges::find(reg.aliases, std::string(brand)) != reg.aliases.end();
		});
	}
	return found;
}

bool ModuleFactory::registerModuleType(std::string_view brand_name,
									   std::string_view module_slug,
									   CreateModuleFunc funcCreate,
									   const ModuleInfoView &info,
									   std::string_view faceplate_filename) {

	auto brand_reg = brand_registry(brand_name);
	auto &brand = (brand_reg != registry().end()) ? *brand_reg : registry().emplace_back(brand_name);
	brand.display_name = brand_name;
	brand.modules[std::string(module_slug)] = {.creation_func = funcCreate,
											   .info = info,
											   .faceplate = std::string{faceplate_filename},
											   .display_name = std::string(module_slug)};
	return true;
}

bool ModuleFactory::registerModuleType(std::string_view typeslug,
									   CreateModuleFunc funcCreate,
									   const ModuleInfoView &info,
									   std::string_view faceplate_filename) {
	return registerModuleType("4msCompany", typeslug, funcCreate, info, faceplate_filename);
}

// brand_module(combined_slug)
// Splits the slug into brand:module
// If no brand: is given, then searches all brands for a matching module slug
static std::pair<std::string_view, std::string_view> brand_module(std::string_view combined_slug) {
	auto colon = combined_slug.find_first_of(':');
	if (colon != std::string_view::npos) {
		return {combined_slug.substr(0, colon), combined_slug.substr(colon + 1)};

	} else {
		auto module_slug = std::string(combined_slug);
		//search all brands for module slug
		for (auto &brand : registry()) {
			if (brand.modules.contains(module_slug)) {
				// pr_dbg("Brand not specified, found %s in %s\n", module_slug.data(), brand.brand_name.c_str());
				return {brand.brand_name.c_str(), combined_slug};
			}
		}
		return {"", ""};
	}
}

static ModuleRegistry *find_module(std::string_view combined_slug) {
	auto [brand, module_name] = brand_module(combined_slug);

	if (auto brand_reg = brand_registry(brand); brand_reg != registry().end()) {
		// Special-case: TSP became BWAVP
		if (module_name.starts_with("TSP")) {
			return &brand_reg->modules[std::string("BWAVP")];
		}
		if (brand_reg->modules.contains(std::string(module_name)))
			return &brand_reg->modules[std::string(module_name)];
	}

	return nullptr;
}

std::unique_ptr<CoreProcessor> ModuleFactory::create(std::string_view combined_slug) {
	if (auto module = find_module(combined_slug)) {
		if (auto f_create = module->creation_func)
			return f_create();
	}

	return nullptr;
}

ModuleInfoView &ModuleFactory::getModuleInfo(std::string_view combined_slug) {
	if (auto module = find_module(combined_slug))
		return module->info;
	else
		return nullinfo;
}

std::string_view ModuleFactory::getModuleFaceplate(std::string_view combined_slug) {
	if (auto module = find_module(combined_slug))
		return module->faceplate;
	else
		return "";
}

std::string_view ModuleFactory::getModuleDisplayName(std::string_view combined_slug) {
	if (auto module = find_module(combined_slug))
		return module->display_name;
	else {
		[[maybe_unused]] auto const &[_, module_name] = brand_module(combined_slug);
		if (module_name.length())
			return module_name;
		else
			return combined_slug;
	}
}

std::string_view ModuleFactory::getModuleSlug(std::string_view brand_slug, std::string_view display_name) {
	if (auto brand_reg = brand_registry(brand_slug); brand_reg != registry().end()) {

		for (auto const &[slug, module] : brand_reg->modules) {
			if (module.display_name == display_name)
				return slug;
		}
	}
	return "";
}

std::string_view ModuleFactory::getBrandDisplayName(std::string_view brand_name) {
	if (auto brand_reg = brand_registry(brand_name); brand_reg != registry().end())
		return brand_reg->display_name;
	else
		return brand_name;
}

std::vector<std::string_view> ModuleFactory::getBrandSlugsWithDisplayName(std::string_view display_name) {
	std::vector<std::string_view> slugs;

	for (auto const &brand : registry()) {
		if (brand.display_name == display_name) {
			slugs.push_back(brand.brand_name);
		}
	}

	return slugs;
}

void ModuleFactory::setModuleDisplayName(std::string_view combined_slug, std::string_view display_name) {
	if (auto module = find_module(combined_slug))
		module->display_name = display_name;
}

void ModuleFactory::setBrandDisplayName(std::string_view brand_name, std::string_view display_name) {
	if (auto brand_reg = brand_registry(brand_name); brand_reg != registry().end()) {
		brand_reg->display_name = display_name;
	}
}

bool ModuleFactory::isValidSlug(std::string_view combined_slug) {
	if (auto module = find_module(combined_slug))
		return bool(module->creation_func);
	else
		return false;
}

bool ModuleFactory::isValidBrandModule(std::string_view brand, std::string_view module_name) {
	if (auto brand_reg = brand_registry(brand); brand_reg != registry().end()) {
		if (brand_reg->modules.contains(std::string(module_name))) {
			return bool(brand_reg->modules[std::string(module_name)].creation_func);
		}
	}

	return false;
}

bool ModuleFactory::isRegisteredBrand(std::string_view brand) {
	return brand_registry(brand) != registry().end();
}

std::vector<std::string_view> ModuleFactory::getAllModuleSlugs(std::string_view brand_slug) {
	std::vector<std::string_view> slugs;

	auto const &modules = brand_registry(brand_slug)->modules;
	slugs.reserve(modules.size());

	for (auto const &[slug, module] : modules) {
		slugs.push_back(slug);
	}

	return slugs;
}

std::vector<std::string> ModuleFactory::getAllModuleDisplayNames(std::string_view brand_display_name) {
	std::vector<std::string> names;

	auto const &modules = brand_registry(brand_display_name)->modules;
	names.reserve(modules.size());

	for (auto const &[slug, module] : modules) {
		names.push_back(module.display_name);
	}

	return names;
}

std::vector<std::string_view> ModuleFactory::getAllBrandDisplayNames() {
	std::vector<std::string_view> brands;
	brands.reserve(registry().size());
	for (auto &brand : registry()) {
		if (std::ranges::find(brands, brand.display_name) == brands.end())
			brands.push_back(brand.display_name);
	}
	return brands;
}

std::vector<std::string_view> ModuleFactory::getAllBrands() {
	std::vector<std::string_view> brands;
	brands.reserve(registry().size());
	for (auto &brand : registry()) {
		brands.emplace_back(brand.brand_name.c_str());
	}
	return brands;
}

bool ModuleFactory::unregisterModule(std::string_view brand, std::string_view module_name) {
	if (auto brand_reg = brand_registry(brand); brand_reg != registry().end()) {
		auto removed = brand_reg->modules.erase(std::string(module_name));

		if (removed && brand_reg->modules.size() == 0) {
			pr_dbg("Brand has no more modules, removing\n");
			registry().remove_if([=](BrandRegistry &reg) { return reg.brand_name == brand; });
		}
		return removed > 0;
	}
	return false;
}

bool ModuleFactory::unregisterBrand(std::string_view brand_name) {
	auto removed = registry().remove_if([=](BrandRegistry &reg) { return reg.brand_name == brand_name; });
	return removed > 0;
}

void ModuleFactory::registerBrandAlias(std::string_view brand_name, std::string_view alias) {
	if (alias.empty() || alias == brand_name)
		return;

	if (auto brand_reg = brand_registry(brand_name); brand_reg != registry().end()) {
		if (std::ranges::find(brand_reg->aliases, alias) != brand_reg->aliases.end())
			return;

		pr_dbg("\t\tBrandAlias{\"%s\", \"%s\"},\n", brand_name.data(), alias.data());
		brand_reg->aliases.emplace_back(alias);
	}
}

bool register_module(std::string_view brand_name,
					 std::string_view typeslug,
					 ModuleFactory::CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename) {

	return ModuleFactory::registerModuleType(brand_name, typeslug, funcCreate, info, faceplate_filename);
}

std::string ModuleFactory::cleanupBrandName(std::string_view brand_name) {
	struct BrandAlias {
		std::string_view to;
		std::string_view from;
	};

	// Manually created list of known aliases
	static constexpr auto fixups = std::array{
		BrandAlias{"4ms-XOXDrums", "4msDrums"},
		BrandAlias{"4ms-XOXDrums", "XOXDrums"},
		BrandAlias{"JWModules", "JW-Modules"},
		BrandAlias{"NANOModules", "NanoModules"},
		BrandAlias{"PhaseOscillator", "InfrasonicAudio"},
		BrandAlias{"StellareModular-TuringMachine", "StellareModular"},
	};

	if (auto f = std::ranges::find(fixups, brand_name, &BrandAlias::from); f != fixups.end())
		return std::string{f->to};
	else
		return std::string{brand_name};
};

} // namespace MetaModule
