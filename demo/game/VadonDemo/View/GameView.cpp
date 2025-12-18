#include <VadonDemo/View/GameView.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>
#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/View/Component.hpp>

#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonApp/Core/Application.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Animation/AnimationSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

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
		auto view_query = component_manager.run_component_query<EntityDirtyTag&, RenderComponent&>();

		VadonDemo::View::View& common_view = m_game_core.get_core().get_view();

		for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
		{
			auto view_render_component = view_it.get_component<RenderComponent>();

			// Make sure the resource is up-to-date
			init_resource(view_render_component->resource);

			// Attempt to redraw based on resource type
			common_view.update_entity_draw_data(ecs_world, view_it.get_entity());

			// Remove the tag
			component_manager.set_entity_tag<EntityDirtyTag>(view_it.get_entity(), false);
		}
	}

	void GameView::update_camera()
	{
		// Also update camera w.r.t player
		// TODO: camera zoom?
		auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&, VadonDemo::View::TransformComponent&>();
		auto player_it = player_query.get_iterator();
		if (player_it.is_valid() == true)
		{
			const auto player_view_transform = player_it.get_component<VadonDemo::View::TransformComponent>();
			Vadon::Render::Canvas::RenderContext& canvas_context = m_game_core.get_render_system().get_canvas_context();

			canvas_context.camera.view_rectangle.position = player_view_transform->position;
		}
	}

	// FIXME: deduplicate between editor and game?
	void GameView::init_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto view_render_component = component_manager.get_component<VadonDemo::View::RenderComponent>(entity);
		if (view_render_component.is_valid() == false)
		{
			return;
		}

		component_manager.set_entity_tag<EntityDirtyTag>(entity, true);
	}

	void GameView::remove_entity(Vadon::ECS::EntityHandle entity)
	{
		// FIXME: this is a hacky solution, need a more efficient way to detect when a projectile expires to then activate its VFX
		remove_projectile_entity(entity);
	}

	void GameView::remove_projectile_entity(Vadon::ECS::EntityHandle entity)
	{
		// FIXME: technically we only want this logic to activate in-game, but ideally it should be in the common View
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// Check if any projectiles with VFX have expired		
		const auto projectile_component = component_manager.get_component<Model::ProjectileComponent>(entity);
		const auto projectile_vfx = component_manager.get_component<VFXComponent>(entity);
		if ((projectile_component.is_valid() == false) || (projectile_vfx.is_valid() == false))
		{
			return;
		}

		if (projectile_component->remaining_lifetime > 0.0f)
		{
			return;
		}

		if (projectile_vfx->vfx_prefab.is_valid() != true)
		{
			return;
		}

		Vadon::Scene::SceneSystem& scene_system = m_game_core.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		const Vadon::Scene::SceneHandle vfx_scene = scene_system.load_scene(projectile_vfx->vfx_prefab);

		Vadon::ECS::EntityHandle vfx_entity = scene_system.instantiate_scene(vfx_scene, ecs_world);

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		entity_manager.set_entity_parent(vfx_entity, m_game_core.get_core().get_model().get_root_entity(ecs_world));

		if (projectile_vfx->lifetime > 0.0f)
		{
			auto timer_component = component_manager.add_component<VFXTimerComponent>(vfx_entity);
			timer_component->remaining_lifetime = projectile_vfx->lifetime;
		}

		auto vfx_view_transform = component_manager.add_component<TransformComponent>(vfx_entity);
		if (vfx_view_transform.is_valid() == true)
		{
			const auto projectile_transform = component_manager.get_component<Model::Transform2D>(entity);
			if (projectile_transform.is_valid() == true)
			{
				vfx_view_transform->position = projectile_transform->position;
			}
		}

		auto vfx_anim_component = component_manager.add_component<AnimationComponent>(vfx_entity);
		if (vfx_anim_component.is_valid() == true)
		{
			if (projectile_vfx->animation.is_valid() == true)
			{
				Vadon::Scene::AnimationSystem& anim_system = m_game_core.get_engine_core().get_system<Vadon::Scene::AnimationSystem>();
				const Vadon::Scene::AnimationHandle anim_handle = anim_system.load_animation(projectile_vfx->animation);

				vfx_anim_component->animation_player.set_animation(m_game_core.get_engine_core(), anim_handle);
			}

			// TODO: asserts to make sure values are valid!
			vfx_anim_component->animation_player.set_looping(vfx_anim_component->looping);
			vfx_anim_component->animation_player.set_time_scale(vfx_anim_component->time_scale);
		}

		m_game_core.get_core().entity_added(ecs_world, vfx_entity);
	}

	void GameView::init_resource(RenderResourceID resource_id)
	{
		if (resource_id.is_valid() == false)
		{
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const RenderResourceHandle resource_handle = resource_system.load_resource(resource_id);

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