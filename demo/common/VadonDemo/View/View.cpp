#include <VadonDemo/View/View.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>
#include <VadonDemo/View/Component.hpp>
#include <VadonDemo/Render/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

namespace
{
	// FIXME: this is used to determine whether we need alpha blending
	// Should we somehow move this to the Render system?
	bool color_has_transparency(const Vadon::Math::ColorRGBA& color)
	{
		return (color.value >> 24) < 0xFF;
	}
}

namespace VadonDemo::View
{
	void View::register_types()
	{
		RenderResource::register_resource();
		Shape::register_resource();
		Sprite::register_resource();

		{
			Vadon::ECS::ComponentRegistry::TagTypeInfo tag_info;
			tag_info.hint_string = "VadonEditor:exclude";
			Vadon::ECS::ComponentRegistry::register_tag_type<EntityDirtyTag>(tag_info);
		}

		TransformComponent::register_component();
		ModelTransformComponent::register_component();
		RenderComponent::register_component();
		AnimationComponent::register_component();
		DamageComponent::register_component();
		VFXComponent::register_component();
		VFXTimerComponent::register_component();
	}

	void View::extract_model_state(Vadon::ECS::World& ecs_world)
	{
		m_prev_model_time = m_current_model_time;
		m_current_model_time = m_core.get_model().get_elapsed_time();

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto transform_query = component_manager.run_component_query<Model::Transform2D&, ModelTransformComponent&>();
		for (auto transform_it = transform_query.get_iterator(); transform_it.is_valid() == true; transform_it.next())
		{
			auto transform_tuple = transform_it.get_tuple();

			const Model::Transform2D& model_transform = std::get<Model::Transform2D&>(transform_tuple);
			ModelTransformComponent& view_model_transform = std::get<ModelTransformComponent&>(transform_tuple);

			if (model_transform.teleported == false)
			{
				view_model_transform.prev_transform = view_model_transform.current_transform;

				// TODO: rotation!
				view_model_transform.current_transform.position = model_transform.position;
				view_model_transform.current_transform.scale = model_transform.scale;
			}
			else
			{
				// TODO: rotation!
				view_model_transform.current_transform.position = model_transform.position;
				view_model_transform.current_transform.scale = model_transform.scale;

				view_model_transform.prev_transform = view_model_transform.current_transform;
			}
		}

		// TODO: any other state to extract?
	}

	void View::lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		const float neg_lerp_factor = 1.0f - lerp_factor;

		// FIXME: is this correct for the view?
		// Floating point precision will probably be horrible...
		const float interpolated_time = (m_current_model_time * lerp_factor) + (m_prev_model_time * neg_lerp_factor);
		const float view_delta_time = interpolated_time - m_last_interpolated_time;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// Interpolate transforms
		{
			auto transform_query = component_manager.run_component_query<TransformComponent&, ModelTransformComponent&>();
			for (auto transform_it = transform_query.get_iterator(); transform_it.is_valid() == true; transform_it.next())
			{
				auto transform_tuple = transform_it.get_tuple();
				TransformComponent& current_transform = std::get<TransformComponent&>(transform_tuple);
				const ModelTransformComponent& model_transform = std::get<ModelTransformComponent&>(transform_tuple);

				current_transform.position = model_transform.current_transform.position * lerp_factor + model_transform.prev_transform.position * neg_lerp_factor;
				current_transform.scale = model_transform.current_transform.scale * lerp_factor + model_transform.prev_transform.scale * neg_lerp_factor;
			}
		}

		// Update animations
		{
			auto anim_query = component_manager.run_component_query<AnimationComponent&, TransformComponent*, RenderComponent*>();
			for (auto anim_it = anim_query.get_iterator(); anim_it.is_valid() == true; anim_it.next())
			{
				auto anim_tuple = anim_it.get_tuple();

				AnimationComponent& anim_component = std::get<AnimationComponent&>(anim_tuple);

				anim_component.animation_player.update(view_delta_time);

				const Vadon::Scene::AnimationSample anim_sample = anim_component.animation_player.get_sample();

				for (const Vadon::Scene::AnimationChannelSample& current_channel : anim_sample.channels)
				{
					// FIXME: have a more appropriate way to link channels to the properties that need to be updated!
					if (current_channel.tag == "radius")
					{
						// TODO: make this compatible with model interpolation?
						// OR assume that this only works for view-only objects?
						TransformComponent* transform_component = std::get<TransformComponent*>(anim_tuple);
						if (transform_component != nullptr)
						{
							transform_component->scale = std::get<float>(current_channel.value);
						}
					}
					else if (current_channel.tag == "opacity")
					{
						RenderComponent* render_component = std::get<RenderComponent*>(anim_tuple);
						if (render_component != nullptr)
						{
							Vadon::Math::ColorVector color_vec = Vadon::Math::ColorRGBA::to_rgba_vector(render_component->color);
							color_vec.a = std::get<float>(current_channel.value);

							render_component->color = Vadon::Math::ColorRGBA::from_rgba_vector(color_vec);

							update_entity_draw_data(ecs_world, anim_it.get_entity());
						}
					}
				}
			}
		}

		// Update VFX timers
		{
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			auto vfx_timer_query = component_manager.run_component_query<VFXTimerComponent&>();
			for (auto vfx_timer_it = vfx_timer_query.get_iterator(); vfx_timer_it.is_valid() == true; vfx_timer_it.next())
			{
				auto vfx_timer_tuple = vfx_timer_it.get_tuple();
				VFXTimerComponent& timer_component = std::get<VFXTimerComponent&>(vfx_timer_tuple);

				timer_component.remaining_lifetime -= view_delta_time;

				if (timer_component.remaining_lifetime <= 0.0f)
				{
					m_core.entity_removed(ecs_world, vfx_timer_it.get_entity());
					entity_manager.remove_entity(vfx_timer_it.get_entity());
				}
			}
		}

		// Update canvas items with new transform
		// FIXME: some other way to do this? Decouple transform updates and Render components?
		{
			auto render_query = component_manager.run_component_query<TransformComponent&, Render::CanvasComponent&>();
			for (auto render_it = render_query.get_iterator(); render_it.is_valid() == true; render_it.next())
			{
				auto render_tuple = render_it.get_tuple();
				const TransformComponent& view_transform = std::get<TransformComponent&>(render_tuple);
				Render::CanvasComponent& canvas_component = std::get<Render::CanvasComponent&>(render_tuple);

				if (canvas_component.canvas_item.is_valid() == false)
				{
					// Canvas item not yet initialized!
					continue;
				}

				// Update canvas item transform
				Vadon::Render::Canvas::Transform canvas_transform;
				canvas_transform.position = view_transform.position;
				canvas_transform.scale = view_transform.scale;

				canvas_system.set_item_transform(canvas_component.canvas_item, canvas_transform);
			}
		}

		m_last_interpolated_time = interpolated_time;
	}

	void View::update_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto component_tuple = component_manager.get_component_tuple<TransformComponent, Model::Transform2D, ModelTransformComponent, Render::CanvasComponent>(view_entity);

		TransformComponent* view_transform = std::get<TransformComponent*>(component_tuple);
		const Model::Transform2D* model_transform = std::get<Model::Transform2D*>(component_tuple);
		ModelTransformComponent* view_model_transform = std::get<ModelTransformComponent*>(component_tuple);

		if ((model_transform != nullptr) && (view_model_transform != nullptr))
		{
			if (view_transform != nullptr)
			{
				// Set transform to that of model
				view_transform->position = model_transform->position;
				view_transform->scale = model_transform->scale;
			}

			// Set both previous and current transform to be the same (to prevent "snapping")
			view_model_transform->prev_transform.position = model_transform->position;
			view_model_transform->prev_transform.scale = model_transform->scale;

			view_model_transform->current_transform = view_model_transform->prev_transform;
		}

		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
		if ((canvas_component == nullptr) || (canvas_component->canvas_item.is_valid() == false))
		{
			// Canvas item not yet initialized
			return;
		}

		Vadon::Render::Canvas::Transform new_transform;
		if (view_transform != nullptr)
		{
			new_transform.position = view_transform->position;
			new_transform.scale = view_transform->scale;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.set_item_transform(canvas_component->canvas_item, new_transform);
	}

	void View::update_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		RenderComponent* render_component = ecs_world.get_component_manager().get_component<RenderComponent>(view_entity);
		if (render_component != nullptr)
		{
			update_entity_draw_data(ecs_world, view_entity, render_component);
		}
	}
	
	void View::remove_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		RenderComponent* view_render_component = component_manager.get_component<RenderComponent>(view_entity);

		if (view_render_component == nullptr)
		{
			// Did not have a render component, so not our concern
			return;
		}

		Render::CanvasComponent* canvas_component = component_manager.get_component<Render::CanvasComponent>(view_entity);
		if (canvas_component == nullptr)
		{
			return;
		}

		if (canvas_component->canvas_item.is_valid() == false)
		{
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_item(canvas_component->canvas_item);
	}

	RenderResourceHandle View::load_render_resource(RenderResourceID resource_id) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		return resource_system.load_resource(resource_id);
	}

	void View::load_resource_data(RenderResourceID resource_id)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const RenderResourceHandle resource_handle = load_render_resource(resource_id);
		RenderResource* view_render_resource = resource_system.get_resource(resource_handle);

		if (view_render_resource->batch.is_valid() == false)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			view_render_resource->batch = canvas_system.create_batch();
			view_render_resource->batch_range = Vadon::Utilities::DataRange();
		}

		if (view_render_resource->batch_range.is_valid() == true)
		{
			// Resource already loaded
			return;
		}

		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
		{
			load_shape_resource(static_cast<Shape*>(view_render_resource));
		}
		else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
		{
			load_sprite_resource(static_cast<Sprite*>(view_render_resource));
		}
		else
		{
			VADON_UNREACHABLE;
		}
	}

	void View::reset_resource_data(RenderResourceID resource_id)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const RenderResourceHandle resource_handle = load_render_resource(resource_id);
		RenderResource* view_render_resource = resource_system.get_resource(resource_handle);
		if (view_render_resource->batch_range.is_valid() == false)
		{
			// Nothing to do
			return;
		}

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_batch(view_render_resource->batch);

		view_render_resource->batch_range = Vadon::Utilities::DataRange();
	}

	View::View(VadonDemo::Core::Core& core)
		: m_core(core)
		, m_random_engine(std::random_device{}())
		, m_texture_dist(0, 3)
		, m_prev_model_time(0.0f)
		, m_current_model_time(0.0f)
		, m_last_interpolated_time(0.0f)
	{
	}

	bool View::initialize()
	{
		// TODO: anything?
		return true;
	}

	void View::global_config_updated()
	{
		// TODO: anything?
	}

	void View::load_shape_resource(Shape* shape)
	{
		VADON_ASSERT(shape->batch_range.is_valid() == false, "Shape already loaded!");

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		if (color_has_transparency(shape->color) == true)
		{
			// Make sure batch sets render state for alpha blend
			Vadon::Render::Canvas::RenderState alpha_blend_state = { .alpha_blend = true };
			canvas_system.set_batch_render_state(shape->batch, alpha_blend_state);
		}

		shape->batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(shape->batch));

		switch (Vadon::Utilities::to_enum<ShapeType>(shape->type))
		{
		case ShapeType::TRIANGLE:
		{
			Vadon::Render::Canvas::Triangle triangle;
			triangle.points[0].position = Vadon::Math::Vector2{0, 0.5f } * shape->radius;
			triangle.points[0].color = shape->color;

			triangle.points[1].position = Vadon::Math::Vector2{ 0.5f , -0.5f } * shape->radius;
			triangle.points[1].color = shape->color;

			triangle.points[2].position = Vadon::Math::Vector2{ -0.5f, -0.5f } * shape->radius;
			triangle.points[2].color = shape->color;

			canvas_system.draw_batch_triangle(shape->batch, triangle);
			shape->batch_range.count = 1;
		}
		break;
		case ShapeType::RECTANGLE:
		{
			Vadon::Render::Canvas::Rectangle box_rectangle;
			box_rectangle.color = shape->color;
			box_rectangle.dimensions.size = Vadon::Math::Vector2{ 1.0f, 1.0f } * shape->radius;
			box_rectangle.filled = true;

			canvas_system.draw_batch_rectangle(shape->batch, box_rectangle);
			shape->batch_range.count = 1;
		}
		break;
		case ShapeType::DIAMOND:
		{
			Vadon::Render::Canvas::Triangle diamond_half_triangle;
			diamond_half_triangle.points[0].position = Vadon::Math::Vector2{ 0, 1.0f } * shape->radius;
			diamond_half_triangle.points[1].position = Vadon::Math::Vector2{ 1.0f, 0 } * shape->radius;
			diamond_half_triangle.points[2].position = Vadon::Math::Vector2{ -1.0f, 0 } * shape->radius;
			
			for (size_t triangle_index = 0; triangle_index < 3; ++triangle_index)
			{
				diamond_half_triangle.points[triangle_index].color = shape->color;
			}

			canvas_system.draw_batch_triangle(shape->batch, diamond_half_triangle);

			diamond_half_triangle.points[0].position = Vadon::Math::Vector2{ -1.0f, 0 } * shape->radius;
			diamond_half_triangle.points[1].position = Vadon::Math::Vector2{ 1.0f, 0 } * shape->radius;
			diamond_half_triangle.points[2].position = Vadon::Math::Vector2{ 0, -1.0f } * shape->radius;

			canvas_system.draw_batch_triangle(shape->batch, diamond_half_triangle);
			shape->batch_range.count = 2;
		}
		break;
		case ShapeType::CIRCLE:
		{
			Vadon::Render::Canvas::Circle circle;
			circle.position = { 0, 0 };
			circle.radius = 0.5f * shape->radius;
			circle.color = shape->color;

			canvas_system.draw_batch_circle(shape->batch, circle);
			shape->batch_range.count = 1;
		}
		break;
		}
	}

	void View::load_sprite_resource(Sprite* sprite)
	{
		VADON_ASSERT(sprite->batch_range.is_valid() == false, "Sprite already loaded!");

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		if (sprite->texture.is_valid() == false)
		{
			// Nothing to draw
			return;
		}

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		VadonDemo::Render::TextureResourceHandle sprite_texture_handle = VadonDemo::Render::TextureResourceHandle::from_resource_handle(resource_system.load_resource(sprite->texture));
		const VadonDemo::Render::TextureResource* sprite_texture = resource_system.get_resource(sprite_texture_handle);
		if (sprite_texture->texture.is_valid() == false)
		{
			// No valid texture loaded
			return;
		}

		// TODO: sprite color to allow transparency, etc?

		sprite->batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(sprite->batch));
		sprite->batch_range.count = 2;

		Vadon::Render::Canvas::Sprite canvas_sprite;
		canvas_sprite.dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.uv_dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.color = Vadon::Math::Color_White;

		canvas_system.set_batch_texture(sprite->batch, Vadon::Render::Canvas::Texture{ .srv = sprite_texture->texture_srv });
		canvas_system.draw_batch_sprite(sprite->batch, canvas_sprite);
	}

	void View::update_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity, RenderComponent* view_render_component)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		Render::CanvasComponent* canvas_component = component_manager.get_component<Render::CanvasComponent>(view_entity);
		if (canvas_component == nullptr)
		{
			return;
		}

		if (canvas_component->canvas_item.is_valid() == false)
		{
			// Nothing to update
			return;
		}

		update_entity_transform(ecs_world, view_entity);

		// Clear the canvas item (we shouldn't show anything if we have no valid data)
		// TODO: show some placeholder instead?
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_item(canvas_component->canvas_item);

		if (view_render_component->resource.is_valid() == false)
		{
			return;
		}

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		RenderResourceHandle view_render_resource_handle = RenderResourceHandle::from_resource_handle(resource_system.load_resource(view_render_component->resource));

		if (view_render_component->color != Vadon::Math::Color_White)
		{
			// Entity has material override, so we add custom draw data
			// FIXME: make this system more flexible?
			set_entity_custom_draw_data(view_render_component, canvas_component, view_render_resource_handle);
			return;
		}

		const RenderResource* view_resource = resource_system.get_resource(view_render_resource_handle);	
		if (view_resource->batch_range.is_valid() == false)
		{
			return;
		}

		// Add batch draw command
		canvas_system.add_item_batch_draw(canvas_component->canvas_item,
			Vadon::Render::Canvas::BatchDrawCommand
			{
				.batch = view_resource->batch,
				.range = view_resource->batch_range
			}
		);
	}

	void View::set_entity_custom_draw_data(RenderComponent* view_render_component, Render::CanvasComponent* canvas_component, RenderResourceHandle view_render_resource_handle)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const RenderResource* view_resource = resource_system.get_resource(view_render_resource_handle);

		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(view_render_resource_handle);
		if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
		{
			set_entity_shape_data(view_render_component, canvas_component, static_cast<const Shape*>(view_resource));
		}
		else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
		{
			set_entity_sprite_data(view_render_component, canvas_component, static_cast<const Sprite*>(view_resource));
		}
		else
		{
			VADON_UNREACHABLE;
		}
	}

	void View::set_entity_shape_data(const RenderComponent* view_render_component, Render::CanvasComponent* canvas_component, const Shape* shape)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		const Vadon::Math::ColorRGBA custom_color = Vadon::Math::ColorRGBA::multiply_colors(shape->color, view_render_component->color);

		if (color_has_transparency(custom_color) == true)
		{
			// Make sure item sets render state for alpha blend
			Vadon::Render::Canvas::RenderState alpha_blend_state = { .alpha_blend = true };
			canvas_system.set_item_render_state(canvas_component->canvas_item, alpha_blend_state);
		}

		switch (Vadon::Utilities::to_enum<ShapeType>(shape->type))
		{
		case ShapeType::TRIANGLE:
		{
			Vadon::Render::Canvas::Triangle triangle;
			triangle.points[0].position = Vadon::Math::Vector2{ 0, 0.5f } * shape->radius;
			triangle.points[0].color = custom_color;

			triangle.points[1].position = Vadon::Math::Vector2{ 0.5f , -0.5f } * shape->radius;
			triangle.points[1].color = custom_color;

			triangle.points[2].position = Vadon::Math::Vector2{ -0.5f, -0.5f } * shape->radius;
			triangle.points[2].color = custom_color;

			canvas_system.draw_item_triangle(canvas_component->canvas_item, triangle);
		}
		break;
		case ShapeType::RECTANGLE:
		{
			Vadon::Render::Canvas::Rectangle box_rectangle;
			box_rectangle.color = custom_color;
			box_rectangle.dimensions.size = Vadon::Math::Vector2{ 1.0f, 1.0f } *shape->radius;
			box_rectangle.filled = true;

			canvas_system.draw_item_rectangle(canvas_component->canvas_item, box_rectangle);
		}
		break;
		case ShapeType::DIAMOND:
		{
			Vadon::Render::Canvas::Triangle diamond_half_triangle;
			diamond_half_triangle.points[0].position = Vadon::Math::Vector2{ 0, 1.0f } *shape->radius;
			diamond_half_triangle.points[1].position = Vadon::Math::Vector2{ 1.0f, 0 } *shape->radius;
			diamond_half_triangle.points[2].position = Vadon::Math::Vector2{ -1.0f, 0 } *shape->radius;

			for (size_t triangle_index = 0; triangle_index < 3; ++triangle_index)
			{
				diamond_half_triangle.points[triangle_index].color = custom_color;
			}

			canvas_system.draw_item_triangle(canvas_component->canvas_item, diamond_half_triangle);

			diamond_half_triangle.points[0].position = Vadon::Math::Vector2{ -1.0f, 0 } *shape->radius;
			diamond_half_triangle.points[1].position = Vadon::Math::Vector2{ 1.0f, 0 } *shape->radius;
			diamond_half_triangle.points[2].position = Vadon::Math::Vector2{ 0, -1.0f } *shape->radius;

			canvas_system.draw_item_triangle(canvas_component->canvas_item, diamond_half_triangle);
		}
		break;
		case ShapeType::CIRCLE:
		{
			Vadon::Render::Canvas::Circle circle;
			circle.position = { 0, 0 };
			circle.radius = 0.5f * shape->radius;
			circle.color = custom_color;

			canvas_system.draw_item_circle(canvas_component->canvas_item, circle);
		}
		break;
		}
	}

	void View::set_entity_sprite_data(const RenderComponent* view_render_component, Render::CanvasComponent* canvas_component, const Sprite* sprite_resource)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		if (sprite_resource->texture.is_valid() == false)
		{
			// Nothing to draw
			return;
		}

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		VadonDemo::Render::TextureResourceHandle sprite_texture_handle = VadonDemo::Render::TextureResourceHandle::from_resource_handle(resource_system.load_resource(sprite_resource->texture));
		const VadonDemo::Render::TextureResource* sprite_texture = resource_system.get_resource(sprite_texture_handle);
		if (sprite_texture->texture.is_valid() == false)
		{
			// No valid texture loaded
			return;
		}

		const Vadon::Math::ColorRGBA custom_color = Vadon::Math::ColorRGBA::multiply_colors(Vadon::Math::Color_White, view_render_component->color);

		if (color_has_transparency(custom_color) == true)
		{
			// Make sure item sets render state for alpha blend
			Vadon::Render::Canvas::RenderState alpha_blend_state = { .alpha_blend = true };
			canvas_system.set_item_render_state(canvas_component->canvas_item, alpha_blend_state);
		}

		Vadon::Render::Canvas::Sprite canvas_sprite;
		canvas_sprite.dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.uv_dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.color = custom_color;

		canvas_system.set_item_texture(canvas_component->canvas_item, Vadon::Render::Canvas::Texture{ .srv = sprite_texture->texture_srv });
		canvas_system.draw_item_sprite(canvas_component->canvas_item, canvas_sprite);
	}
}