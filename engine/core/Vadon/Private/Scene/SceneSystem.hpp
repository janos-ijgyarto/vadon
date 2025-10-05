#ifndef VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#define VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Private/Scene/Scene.hpp>

#include <Vadon/Private/Scene/Animation/AnimationSystem.hpp>

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

		SceneHandle find_scene(SceneID scene_id) const override;

		SceneHandle load_scene(SceneID scene_id) override;

		bool package_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity) override;

		ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene) override;

		bool is_scene_dependent(SceneID base_scene_id, SceneID dependent_scene_id) override;
	protected:
		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		bool parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<SceneID>& dependency_stack);
		bool internal_is_scene_dependent(SceneID scene_id, std::vector<SceneID>& dependency_stack);

		const Scene* get_scene(SceneHandle scene_handle) const;
		Scene* get_scene(SceneHandle scene_handle) { return const_cast<Scene*>(std::as_const(*this).get_scene(scene_handle)); }

		AnimationSystem m_animation_system;

		friend Core::EngineCore;
	};
}
#endif