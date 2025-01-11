#ifndef VADONDEMO_VIEW_VIEW_HPP
#define VADONDEMO_VIEW_VIEW_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <VadonDemo/View/Resource.hpp>
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
		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, float lerp_factor);

		VADONDEMO_API Vadon::Render::Canvas::LayerHandle get_canvas_layer() const;
		VADONDEMO_API void update_view_resource(ViewResourceHandle resource_handle);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif