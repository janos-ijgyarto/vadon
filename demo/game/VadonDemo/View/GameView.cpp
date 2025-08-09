#include <VadonDemo/View/GameView.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/View/Component.hpp>

#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonApp/Core/Application.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::View
{
	GameView::~GameView() = default;

	GameView::GameView(Core::GameCore& core)
		: m_game_core(core)
	{ }

	bool GameView::initialize()
	{
		m_game_core.get_core().add_entity_event_callback(
			[this](Vadon::ECS::World& /*ecs_world*/, const VadonDemo::Core::EntityEvent& event)
			{
				switch (event.type)
				{
				case VadonDemo::Core::EntityEventType::ADDED:
					init_entity(event.entity);
					break;
				case VadonDemo::Core::EntityEventType::REMOVED:
					remove_entity(event.entity);
					break;
				}
			}
		);

		return true;
	}

	void GameView::update()
	{
		update_dirty_entities();

		Model::GameModel& game_model = m_game_core.get_model();
		View& view = m_game_core.get_core().get_view();
		if (game_model.is_updated() == true)
		{
			// Model was updated, so we have to extract the state for the view
			view.extract_model_state(m_game_core.get_ecs_world());
		}

		// Interpolate the view state based on model accumulator
		view.lerp_view_state(m_game_core.get_ecs_world(), game_model.get_accumulator() / game_model.get_sim_timestep());

		update_camera();

		// Update frame counter
		++m_view_frame_count;
	}

	void GameView::update_dirty_entities()
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto view_query = component_manager.run_component_query<ViewEntityDirtyTag&, ViewComponent&>();

		VadonDemo::View::View& common_view = m_game_core.get_core().get_view();

		for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
		{
			auto view_tuple = view_it.get_tuple();
			ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

			// Make sure the resource is up-to-date
			init_resource(current_view_component.resource);

			// Attempt to redraw based on resource type
			common_view.update_entity_draw_data(ecs_world, view_it.get_entity());

			// Remove the tag
			component_manager.set_entity_tag<ViewEntityDirtyTag>(view_it.get_entity(), false);
		}
	}

	void GameView::update_camera()
	{
		// Also update camera w.r.t player
		// TODO: camera zoom?
		auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&, VadonDemo::View::ViewComponent&>();
		auto player_it = player_query.get_iterator();
		if (player_it.is_valid() == true)
		{
			auto player_components = player_it.get_tuple();

			Model::GameModel& game_model = m_game_core.get_model();
			const float lerp_factor = game_model.get_accumulator() / game_model.get_sim_timestep();

			const VadonDemo::View::ViewComponent& player_view_component = std::get<VadonDemo::View::ViewComponent&>(player_components);
			Vadon::Render::Canvas::RenderContext& canvas_context = m_game_core.get_render_system().get_canvas_context();

			canvas_context.camera.view_rectangle.position = player_view_component.prev_transform.position * (1.0f - lerp_factor) + player_view_component.current_transform.position * lerp_factor;
		}
	}

	// FIXME: deduplicate between editor and game?
	void GameView::init_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);
		if (view_component == nullptr)
		{
			return;
		}

		component_manager.set_entity_tag<ViewEntityDirtyTag>(entity, true);
	}

	void GameView::remove_entity(Vadon::ECS::EntityHandle /*entity*/)
	{
		// TODO: anything?
	}

	void GameView::init_resource(ViewResourceID resource_id)
	{
		if (resource_id.is_valid() == false)
		{
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const ViewResourceHandle resource_handle = resource_system.load_resource(resource_id);

		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
		{
			// Try to load the texture for the sprite
			const Sprite* sprite_resource = resource_system.get_resource(SpriteResourceHandle::from_resource_handle(resource_handle));
			m_game_core.get_render_system().load_texture_resource(sprite_resource->texture);
		}

		VadonDemo::View::View& common_view = m_game_core.get_core().get_view();
		common_view.load_resource_data(resource_id);
	}
}