#ifndef VADON_SCENE_MODULE_HPP
#define VADON_SCENE_MODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Scene
{
	class SceneModule : public Core::SystemModule<Core::SystemModuleList, SceneModule, class SceneSystem, class ResourceSystem>
	{
	};

	template<typename SysImpl>
	using SceneSystemBase = Core::System<SceneModule, SysImpl>;
}
#endif