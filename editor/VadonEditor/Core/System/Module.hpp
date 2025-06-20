#ifndef VADONEDITOR_CORE_SYSTEM_MODULE_HPP
#define VADONEDITOR_CORE_SYSTEM_MODULE_HPP
#include <Vadon/Utilities/System/Singleton/Module.hpp>
namespace VadonEditor::Model
{
	class ModelModule;
}
namespace VadonEditor::Platform
{
	class PlatformModule;
}
namespace VadonEditor::UI
{
	class UIModule;
}
namespace VadonEditor::View
{
	class ViewModule;
}
namespace VadonEditor::Core
{
	class CoreModule;

	template<typename ModuleList, typename ModuleImpl, typename... Systems>
	using SystemModule = Vadon::Utilities::SingletonModule<ModuleList, ModuleImpl, Systems...>;

	using SystemModuleList = Vadon::Utilities::TypeList<CoreModule, Model::ModelModule, Platform::PlatformModule, UI::UIModule, View::ViewModule>;
}
#endif