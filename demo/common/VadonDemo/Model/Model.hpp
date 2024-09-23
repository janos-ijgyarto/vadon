#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
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
namespace Vadon::Utilities
{
	class PacketQueue;
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

		VADONDEMO_API void render_sync(Vadon::ECS::World& ecs_world);

		VADONDEMO_API void update_view_async(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& view_update_queue);
		VADONDEMO_API void render_async(const Vadon::Utilities::PacketQueue& render_queue);
		VADONDEMO_API void lerp_view_state(float lerp_weight);

		VADONDEMO_API void reset_transforms(Vadon::ECS::World& ecs_world);

		VADONDEMO_API void update_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);

		VADONDEMO_API Vadon::Render::Canvas::LayerHandle get_canvas_layer() const;

		VADONDEMO_API static void init_engine_environment(Vadon::Core::EngineEnvironment& environment);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif