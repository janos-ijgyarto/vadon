#ifndef VADON_SCENE_SCENESYSTEM_HPP
#define VADON_SCENE_SCENESYSTEM_HPP
#include <Vadon/Core/File/RootDirectory.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Scene.hpp>
namespace Vadon::ECS
{
	class World;
}
namespace Vadon::Scene
{
	class SceneSystem : public SceneSystemBase<SceneSystem>
	{
	public:
		// FIXME: should only allow this system to create scenes?
		// Technically could create in ResourceSystem directly, which will cause UB
		virtual SceneHandle create_scene() = 0;

		virtual SceneHandle find_scene(ResourceID scene_id) const = 0;

		virtual bool package_scene_data(SceneHandle scene_handle, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle root_entity) = 0;

		virtual bool save_scene(SceneHandle scene_handle) = 0;
		virtual bool load_scene(SceneHandle scene_handle) = 0;
		virtual Vadon::ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, Vadon::ECS::World& ecs_world, bool is_sub_scene = true) = 0;

		// FIXME: this function is currently non-const because we have to load the scene to check dependencies
		// Should either cache deps or be able to load it separately when registering scene resources!
		// Recursively checks contents of second argument whether it is a dependent of the first argument
		// (i.e whether instantiating any of the contents of the second arg requires the first arg)
		virtual bool is_scene_dependent(SceneHandle base_scene_handle, SceneHandle dependent_scene_handle) = 0;
	protected:
		SceneSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif