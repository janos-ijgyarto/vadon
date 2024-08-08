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
		ResourceHandle create_scene() override;
		bool set_scene_data(ResourceHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity) override;
		ECS::EntityHandle instantiate_scene(ResourceHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene) override;

		bool is_scene_dependent(ResourceHandle scene_handle, ResourceHandle dependent_scene_handle) const override;
	protected:
		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		bool parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<ResourceHandle>& dependency_stack);
		bool internal_is_scene_dependent(ResourceHandle scene_handle, std::vector<ResourceHandle>& dependency_stack) const;

		static bool serialize_scene(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, ResourceBase& resource);

		ResourceSystem m_resource_system;

		friend Core::EngineCore;
	};
}
#endif