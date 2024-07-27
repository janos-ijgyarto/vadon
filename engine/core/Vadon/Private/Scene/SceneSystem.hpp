#ifndef VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#define VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Private/Scene/Scene.hpp>

#include <Vadon/Private/ECS/Entity/Entity.hpp>
#include <Vadon/Private/ECS/World/World.hpp>

#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

namespace Vadon::Private::Core
{
	class EngineCore;
}
namespace Vadon::Private::Scene
{
	class SceneSystem final : public Vadon::Scene::SceneSystem
	{
	public:
		SceneHandle create_scene(SceneInfo scene_info) override;
		bool is_scene_valid(SceneHandle scene_handle) const override { return m_scene_pool.is_handle_valid(scene_handle); }
		void remove_scene(SceneHandle scene_handle) override;

		SceneInfo get_scene_info(SceneHandle scene_handle) const override;

		bool set_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity) override;
		ECS::EntityHandle instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world) override;

		bool serialize_scene(SceneHandle scene_handle, Vadon::Utilities::Serializer& serializer) override;
	protected:
		using ScenePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Scene, SceneData>;

		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		ScenePool m_scene_pool;

		ResourceSystem m_resource_system;

		friend Core::EngineCore;
	};
}
#endif