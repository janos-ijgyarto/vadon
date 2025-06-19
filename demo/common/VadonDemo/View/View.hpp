#ifndef VADONDEMO_VIEW_VIEW_HPP
#define VADONDEMO_VIEW_VIEW_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <random>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class Core;
}
namespace VadonDemo::Model
{
	struct Transform2D;
}
namespace VadonDemo::Render
{
	struct CanvasComponent;
}
namespace VadonDemo::View
{
	struct ViewComponent;

	class View
	{
	public:
		static void register_types();

		VADONDEMO_API void extract_model_state(Vadon::ECS::World& ecs_world);
		VADONDEMO_API void lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor);

		// TODO: allow these to branch on what data needs updating!
		VADONDEMO_API void update_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity);
		VADONDEMO_API void update_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity);
		VADONDEMO_API void remove_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity);

		VADONDEMO_API void load_resource(ViewResourceHandle resource_handle);
		VADONDEMO_API void reset_resource(ViewResourceHandle resource_handle);
	private:
		View(VadonDemo::Core::Core& core);

		bool initialize();
		void global_config_updated();

		void load_shape_resource(Shape* shape);
		void load_sprite_resource(Sprite* sprite_resource);

		void update_entity_transform(const Model::Transform2D* transform, ViewComponent* view_component, Render::CanvasComponent* canvas_component);
		void update_entity_draw_data(const Model::Transform2D* transform, ViewComponent* view_component, Render::CanvasComponent* canvas_component);

		VadonDemo::Core::Core& m_core;

		std::mt19937 m_random_engine;
		std::uniform_int_distribution<int> m_texture_dist;

		friend Core::Core;
	};
}
#endif