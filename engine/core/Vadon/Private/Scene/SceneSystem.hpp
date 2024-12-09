#ifndef VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#define VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Private/Scene/Scene.hpp>

#include <Vadon/Private/ECS/Entity/Entity.hpp>
#include <Vadon/Private/ECS/World/World.hpp>

namespace Vadon::Private::Core
{
	class EngineCore;
}
namespace Vadon::Private::Scene
{
	class SceneSystem final : public Vadon::Scene::SceneSystem
	{
	public:		
		SceneHandle create_scene() override;

		SceneHandle find_scene(ResourceID scene_id) const override;

		bool package_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity) override;

		ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene) override;

		bool is_scene_dependent(SceneHandle base_scene_handle, SceneHandle dependent_scene_handle) override;
	protected:
		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		bool parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<SceneHandle>& dependency_stack);
		bool internal_is_scene_dependent(SceneHandle scene_handle, std::vector<SceneHandle>& dependency_stack);

		friend Core::EngineCore;
	};
}
#endif