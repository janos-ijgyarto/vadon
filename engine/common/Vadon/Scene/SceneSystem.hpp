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
		virtual SceneHandle create_scene(SceneInfo scene_info) = 0;
		virtual bool is_scene_valid(SceneHandle scene_handle) const = 0;
		virtual void remove_scene(SceneHandle scene_handle) = 0;

		virtual SceneInfo get_scene_info(SceneHandle scene_handle) const = 0;

		virtual bool set_scene_data(SceneHandle scene_handle, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle root_entity) = 0;
		virtual Vadon::ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, Vadon::ECS::World& ecs_world) = 0;

		virtual bool serialize_scene(SceneHandle scene_handle, Vadon::Utilities::Serializer& serializer) = 0;
	protected:
		SceneSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif