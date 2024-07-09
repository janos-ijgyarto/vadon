#ifndef VADON_CORE_SYSTEM_SYSTEMMODULE_HPP
#define VADON_CORE_SYSTEM_SYSTEMMODULE_HPP
#include <Vadon/Utilities/System/Singleton/Module.hpp>
namespace Vadon::Render
{
	class RenderModule;
	class GraphicsModule;
}
namespace Vadon::Scene
{
	class SceneModule;
}
namespace Vadon::Core
{
	template<typename ModuleList, typename ModuleImpl, typename... Types>
	using SystemModule = Utilities::SingletonModule<ModuleList, ModuleImpl, Types...>;

	using SystemModuleList = Vadon::Utilities::TypeList<class CoreModule, Render::RenderModule, Render::GraphicsModule, Scene::SceneModule>;
}
#endif