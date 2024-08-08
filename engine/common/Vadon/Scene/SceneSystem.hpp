#ifndef VADON_SCENE_SCENESYSTEM_HPP
#define VADON_SCENE_SCENESYSTEM_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>
namespace Vadon::ECS
{
	class World;
}
namespace Vadon::Scene
{
	class SceneSystem : public SceneSystemBase<SceneSystem>
	{
	public:
		virtual ResourceHandle create_scene() = 0;
		virtual bool set_scene_data(ResourceHandle scene_handle, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle root_entity) = 0;
		virtual Vadon::ECS::EntityHandle instantiate_scene(ResourceHandle scene_handle, Vadon::ECS::World& ecs_world, bool is_sub_scene = true) = 0;

		virtual bool is_scene_dependent(ResourceHandle scene_handle, ResourceHandle dependent_scene_handle) const = 0;
	protected:
		SceneSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif