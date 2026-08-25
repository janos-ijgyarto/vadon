#ifndef VADON_MODEL_MODULE_HPP
#define VADON_MODEL_MODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Model
{
	class ModelSystemModule : public Core::SystemModule<Core::SystemModuleList, ModelSystemModule, class SceneSystem, class AnimationSystem, class ResourceSystem>
	{
	};

	template<typename SysImpl>
	using ModelSystemBase = Core::System<ModelSystemModule, SysImpl>;
}
#endif