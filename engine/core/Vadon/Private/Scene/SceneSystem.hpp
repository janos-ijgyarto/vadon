#ifndef VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#define VADON_PRIVATE_SCENE_SCENESYSTEM_HPP
#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Private/Scene/Scene.hpp>
#include <Vadon/Private/Scene/Node/Node.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
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

		bool set_scene_data(SceneHandle scene_handle, Node& root_node) override;
		Node* instantiate_scene(SceneHandle scene_handle) override;

		bool save_scene(SceneHandle scene_handle, Utilities::JSON& writer) override;
		bool load_scene(SceneHandle scene_handle, Utilities::JSONReader& reader) override;

		void update(float delta_time) override;
	protected:
		using ScenePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Scene, SceneData>;

		SceneSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		ScenePool m_scene_pool;

		friend Core::EngineCore;
	};
}
#endif