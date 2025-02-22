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
		if (init_canvas() == false)
		{
			return false;
		}

		return true;
	}

	void GameView::update()
	{
		Model::GameModel& game_model = m_game_core.get_model();
		View& view = m_game_core.get_core().get_view();
		if (game_model.is_updated() == true)
		{
			// Model was updated, so we have to extract the state for the view
			view.extract_model_state(m_game_core.get_ecs_world());
		}

		if (m_deferred_init_queue.empty() == false)
		{
			for (Vadon::ECS::EntityHandle current_entity : m_deferred_init_queue)
			{
				init_entity(current_entity);
			}
			m_deferred_init_queue.clear();
		}

		// Interpolate the view state based on model accumulator
		view.lerp_view_state(m_game_core.get_ecs_world(), game_model.get_accumulator() / game_model.get_sim_timestep());

		update_camera();

		// Update frame counter
		++m_view_frame_count;
	}

	bool GameView::init_canvas()
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		component_manager.register_event_callback<VadonDemo::View::ViewComponent>(
			[this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
			{
				switch (component_event.event_type)
				{
				case Vadon::ECS::ComponentEventType::ADDED:
					m_deferred_init_queue.push_back(component_event.owner);
					break;
					// TODO: clean up view resources
				}
			}
		);

		return true;
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
		Vadon::ECS::ComponentManager& component_manager = m_game_core.get_ecs_world().get_component_manager();
		VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);
		VADON_ASSERT(view_component != nullptr, "Failed to get view component!");

		if (view_component->resource.is_valid() == true)
		{
			init_resource(view_component->resource);
		}

		m_game_core.get_core().get_view().update_entity_draw_data(ecs_world, entity);
	}

	void GameView::init_resource(VadonDemo::View::ViewResourceHandle resource_handle)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

		if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::Sprite>(), resource_info.type_id))
		{
			load_sprite_resource(VadonDemo::View::SpriteResourceHandle::from_resource_handle(resource_handle));
		}

		m_game_core.get_core().get_view().update_resource(m_game_core.get_ecs_world(), resource_handle);
	}

	void GameView::load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
	{
		// TODO: implement general system for loading textures as resources!
		Vadon::Scene::ResourceSystem& resource_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

		if (sprite_resource->texture_srv.is_valid() == true)
		{
			// Only try to load once, assume it already has correct data
			return;
		}

		Render::TextureResource* sprite_texture = m_game_core.get_render_system().get_texture_resource(sprite_resource->texture_path);
		VADON_ASSERT(sprite_texture != nullptr, "Cannot load texture!");
		sprite_resource->texture_srv = sprite_texture->srv;
	}
}