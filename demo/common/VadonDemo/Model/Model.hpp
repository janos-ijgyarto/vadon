#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineCoreInterface;
	struct EngineEnvironment;
}
namespace Vadon::ECS
{
	class World;
	struct ComponentEvent;
}
namespace VadonDemo::Model
{
	class Model
	{
	public:
		VADONDEMO_API Model(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~Model();

		VADONDEMO_API bool initialize();

		// FIXME: view logic should not be in model!
		// Ideally we will have ECS systems which are initialized and run by the relevant game systems
		VADONDEMO_API void init_simulation(Vadon::ECS::World& ecs_world);
		VADONDEMO_API void update_simulation(Vadon::ECS::World& ecs_world, float delta_time);
		VADONDEMO_API void update_rendering(Vadon::ECS::World& ecs_world);

		VADONDEMO_API Vadon::Render::Canvas::LayerHandle get_canvas_layer() const;

		VADONDEMO_API void component_event(Vadon::ECS::World& ecs_world, const Vadon::ECS::ComponentEvent& event);

		VADONDEMO_API static void init_engine_environment(Vadon::Core::EngineEnvironment& environment);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif