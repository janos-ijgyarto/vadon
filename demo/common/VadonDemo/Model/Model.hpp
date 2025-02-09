#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineCoreInterface;
	class EngineEnvironment;
}
namespace Vadon::ECS
{
	class World;
	struct ComponentEvent;
}
namespace VadonDemo::Model
{
	struct PlayerInput;

	class Model
	{
	public:
		VADONDEMO_API Model(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~Model();

		static void register_types();
		VADONDEMO_API bool initialize();

		VADONDEMO_API bool init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);
		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, float delta_time);
		VADONDEMO_API bool is_in_end_state(Vadon::ECS::World& ecs_world) const;
		VADONDEMO_API void end_simulation(Vadon::ECS::World& ecs_world);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif