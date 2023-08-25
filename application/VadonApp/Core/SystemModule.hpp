#ifndef VADONAPP_CORE_SYSTEMMODULE_HPP
#define VADONAPP_CORE_SYSTEMMODULE_HPP
#include <Vadon/Core/System/SystemModule.hpp>
namespace VadonApp::Platform
{
	class PlatformModule;
}
namespace VadonApp::UI
{
	class UIModule;
}
namespace VadonApp::Core
{
	template<typename ModuleList, typename ModuleImpl, typename... Systems>
	using SystemModule = Vadon::Core::SystemModule<ModuleList, ModuleImpl, Systems...>;

	using SystemModuleList = Vadon::Utilities::TypeList<class Platform::PlatformModule, class UI::UIModule>;
}
#endif