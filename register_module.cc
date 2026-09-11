#include "CoreModules/moduleFactory.hh"

namespace MetaModule
{

using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

bool register_module(std::string_view brand_name,
					 std::string_view typeslug,
					 ModuleFactory::CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename) {

	return ModuleFactory::registerModuleType(brand_name, typeslug, funcCreate, info, faceplate_filename);
}

bool register_context_menu(std::string_view brand_slug, std::string_view module_slug, ContextMenuHandlers handlers) {
	return ModuleFactory::registerContextMenu(brand_slug, module_slug, std::move(handlers));
}

} // namespace MetaModule
