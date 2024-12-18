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
		// NOTE: these are convenience functions, equivalent to calling ResourceSystem directly
		virtual SceneHandle create_scene() = 0;
		virtual SceneHandle find_scene(ResourceID scene_id) const = 0;

		virtual SceneHandle load_scene(ResourceID scene_id) = 0;

		virtual bool package_scene_data(SceneHandle scene_handle, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle root_entity) = 0;

		// NOTE: we don't check for circular dependencies, by this stage we assume scenes contain valid data
		virtual Vadon::ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, Vadon::ECS::World& ecs_world, bool is_sub_scene = true) = 0;
		
		// NOTE: cannot be const because we have to load the scenes to check for dependencies
		// We expect to have to load them anyway to instantiate
		virtual bool is_scene_dependent(SceneHandle base_scene_handle, SceneHandle dependent_scene_handle) = 0;
	protected:
		SceneSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif