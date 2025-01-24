#ifndef VADONDEMO_VIEW_VIEW_HPP
#define VADONDEMO_VIEW_VIEW_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::View
{
	class View
	{
	public:
		VADONDEMO_API View(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~View();

		VADONDEMO_API bool initialize();

		VADONDEMO_API bool init_visualization(Vadon::ECS::World& ecs_world);

		VADONDEMO_API void extract_model_state(Vadon::ECS::World& ecs_world);
		VADONDEMO_API void lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor);

		// TODO: allow these to branch on what data needs updating!
		VADONDEMO_API void update_view_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity);
		VADONDEMO_API void update_view_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity);
		VADONDEMO_API void update_view_resource(Vadon::ECS::World& ecs_world, ViewResourceHandle resource_handle);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif