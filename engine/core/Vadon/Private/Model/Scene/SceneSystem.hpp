#ifndef VADON_PRIVATE_MODEL_SCENE_SCENESYSTEM_HPP
#define VADON_PRIVATE_MODEL_SCENE_SCENESYSTEM_HPP
#include <Vadon/Model/Scene/SceneSystem.hpp>
#include <Vadon/Private/Model/Scene/Scene.hpp>

#include <Vadon/Private/Model/Animation/AnimationSystem.hpp>

#include <Vadon/Private/ECS/Entity/Entity.hpp>
#include <Vadon/Private/ECS/World/World.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace Vadon::Private::Core
{
	class EngineCore;
}
namespace Vadon::Private::Model
{
	class SceneSystem final : public Vadon::Model::SceneSystem
	{
	public:		
		SceneHandle create_scene() override;

		SceneHandle find_scene(SceneID scene_id) const override;

		SceneHandle load_scene(SceneID scene_id) override;

		ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world) override;

		bool is_scene_dependent(SceneID base_scene_id, SceneID dependent_scene_id) override;
	protected:
		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		static void register_types();
		static void register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);

		bool initialize();
		void shutdown();

		bool internal_is_scene_dependent(SceneID scene_id, std::vector<SceneID>& dependency_stack);

		ECS::EntityHandle internal_instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, const SceneID& parent_scene_id);

		const Scene* get_scene(SceneHandle scene_handle) const;
		Scene* get_scene(SceneHandle scene_handle) { return const_cast<Scene*>(std::as_const(*this).get_scene(scene_handle)); }

		AnimationSystem m_animation_system;

		friend Core::EngineCore;
	};
}
#endif