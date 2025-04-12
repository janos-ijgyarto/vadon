#include <VadonDemo/View/View.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/View/Component.hpp>
#include <VadonDemo/Render/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::View
{
	void View::register_types()
	{
		ViewResource::register_resource();
		Shape::register_resource();
		Sprite::register_resource();

		ViewComponent::register_component();
	}

	void View::extract_model_state(Vadon::ECS::World& ecs_world)
	{
		// TODO: any other state to extract?
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto view_query = component_manager.run_component_query<Model::Transform2D&, ViewComponent&>();
		for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
		{
			auto view_tuple = view_it.get_tuple();

			const Model::Transform2D& model_transform = std::get<Model::Transform2D&>(view_tuple);
			ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

			current_view_component.prev_transform = current_view_component.current_transform;

			// TODO: rotation!
			current_view_component.current_transform.position = model_transform.position;
			current_view_component.current_transform.scale = model_transform.scale;
		}
	}

	void View::lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		const float neg_lerp_factor = 1.0f - lerp_factor;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto view_query = component_manager.run_component_query<ViewComponent&, Render::CanvasComponent&>();
		for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
		{
			auto view_tuple = view_it.get_tuple();
			ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);
			Render::CanvasComponent& canvas_component = std::get<Render::CanvasComponent&>(view_tuple);

			if (canvas_component.canvas_item.is_valid() == false)
			{
				// Canvas item not yet initialized!
				continue;
			}

			// Update Item with interpolated transform
			Vadon::Render::Canvas::Transform interpolated_transform;
			interpolated_transform.position = current_view_component.current_transform.position * lerp_factor + current_view_component.prev_transform.position * neg_lerp_factor;
			interpolated_transform.scale = current_view_component.current_transform.scale;

			canvas_system.set_item_transform(canvas_component.canvas_item, interpolated_transform);
		}
	}

	void View::update_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Model::Transform2D, ViewComponent, Render::CanvasComponent>(view_entity);

		ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);
		if (view_component == nullptr)
		{
			return;
		}
		const Model::Transform2D* transform = std::get<Model::Transform2D*>(component_tuple);
		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
		update_entity_transform(transform, view_component, canvas_component);
	}

	void View::update_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Model::Transform2D, ViewComponent, Render::CanvasComponent>(view_entity);

		ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);

		if (view_component == nullptr)
		{
			return;
		}

		const Model::Transform2D* transform = std::get<Model::Transform2D*>(component_tuple);
		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
		update_entity_draw_data(transform, view_component, canvas_component);
	}
	
	void View::remove_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		auto component_tuple = ecs_world.get_component_manager().get_component_tuple<ViewComponent, Render::CanvasComponent>(view_entity);

		ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);
		if (view_component == nullptr)
		{
			return;
		}

		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
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

	void View::update_resource(Vadon::ECS::World& ecs_world, ViewResourceHandle resource_handle)
	{
		CanvasResource* resource = find_resource(resource_handle);
		if (resource != nullptr)
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
			if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
			{
				update_shape_resource(ShapeResourceHandle::from_resource_handle(resource_handle), *resource);
			}
			else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
			{
				update_sprite_resource(SpriteResourceHandle::from_resource_handle(resource_handle), *resource);
			}
		}
		else
		{
			resource = &get_resource(resource_handle);
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		// Find all entities that use this resource and re-initialize their draw data
		// FIXME: could do this deferred so we only have to iterate over the entities once
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto view_query = component_manager.run_component_query<ViewComponent&, Render::CanvasComponent&>();
		for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
		{
			auto view_tuple = view_it.get_tuple();
			ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

			if (current_view_component.resource != resource_handle)
			{
				continue;
			}

			Render::CanvasComponent& canvas_component = std::get<Render::CanvasComponent&>(view_tuple);
			if (canvas_component.canvas_item.is_valid() == false)
			{
				// Entity not yet initialized
				continue;
			}

			// FIXME: duplicating draw commands across items, need to update each one that uses this resource
			// Could implement additional layers of indirection:
			// Allow a batch to record batch draw commands, so one batch can be used as "template"
			// Then all items can just reference the batch with a "draw all" setting
			canvas_system.clear_item(canvas_component.canvas_item);
			if (resource->batch_range.count > 0)
			{
				canvas_system.add_item_batch_draw(canvas_component.canvas_item,
					Vadon::Render::Canvas::BatchDrawCommand
					{
						.batch = resource->batch,
						.range = resource->batch_range
					}
				);
			}
		}
	}

	View::View(VadonDemo::Core::Core& core)
		: m_core(core)
		, m_random_engine(std::random_device{}())
		, m_texture_dist(0, 3)
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

	View::CanvasResource* View::find_resource(ViewResourceHandle view_resource_handle)
	{
		auto resource_it = m_canvas_resource_lookup.find(view_resource_handle.to_uint());
		if (resource_it != m_canvas_resource_lookup.end())
		{
			return &resource_it->second;
		}

		return nullptr;
	}

	View::CanvasResource& View::get_resource(ViewResourceHandle view_resource_handle)
	{
		CanvasResource* resource = find_resource(view_resource_handle);
		if (resource != nullptr)
		{
			return *resource;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(view_resource_handle);
		if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
		{
			return add_shape_resource(ShapeResourceHandle::from_resource_handle(view_resource_handle));
		}
		else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
		{
			return add_sprite_resource(SpriteResourceHandle::from_resource_handle(view_resource_handle));
		}

		VADON_UNREACHABLE;
	}

	View::CanvasResource& View::add_canvas_resource(ViewResourceHandle view_resource_handle)
	{
		CanvasResource& canvas_resource = m_canvas_resource_lookup[view_resource_handle.to_uint()];

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_resource.batch = canvas_system.create_batch();

		return canvas_resource;
	}

	View::CanvasResource& View::add_shape_resource(ShapeResourceHandle shape_handle)
	{
		CanvasResource& canvas_resource = add_canvas_resource(ViewResourceHandle::from_resource_handle(shape_handle));
		update_shape_resource(shape_handle, canvas_resource);

		return canvas_resource;
	}

	void View::update_shape_resource(ShapeResourceHandle shape_handle, CanvasResource& shape_canvas_resource)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const Shape* shape = resource_system.get_resource(shape_handle);

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		canvas_system.clear_batch(shape_canvas_resource.batch);

		shape_canvas_resource.batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(shape_canvas_resource.batch));

		switch (Vadon::Utilities::to_enum<ShapeType>(shape->type))
		{
		case ShapeType::TRIANGLE:
		{
			Vadon::Render::Canvas::Triangle triangle;
			triangle.color = Vadon::Render::Canvas::ColorRGBA(shape->color, 1.0f);

			triangle.point_a.position = { 0, 0.5f };
			triangle.point_b.position = { 0.5f, -0.5f };
			triangle.point_c.position = { -0.5f, -0.5f };

			canvas_system.draw_batch_triangle(shape_canvas_resource.batch, triangle);
			shape_canvas_resource.batch_range.count = 1;
		}
		break;
		case ShapeType::RECTANGLE:
		{
			Vadon::Render::Canvas::Rectangle box_rectangle;
			box_rectangle.color = Vadon::Render::Canvas::ColorRGBA(shape->color, 1.0f);
			box_rectangle.dimensions.size = { 1.0f, 1.0f };
			box_rectangle.filled = true;

			canvas_system.draw_batch_rectangle(shape_canvas_resource.batch, box_rectangle);
			shape_canvas_resource.batch_range.count = 1;
		}
		break;
		case ShapeType::DIAMOND:
		{
			Vadon::Render::Canvas::Triangle diamond_half_triangle;
			diamond_half_triangle.color = Vadon::Render::Canvas::ColorRGBA(shape->color, 1.0f);

			diamond_half_triangle.point_a.position = { 0, 1.0f };
			diamond_half_triangle.point_b.position = { 1.0f, 0 };
			diamond_half_triangle.point_c.position = { -1.0f, 0 };

			canvas_system.draw_batch_triangle(shape_canvas_resource.batch, diamond_half_triangle);

			diamond_half_triangle.point_a.position = { -1.0f, 0 };
			diamond_half_triangle.point_b.position = { 1.0f, 0 };
			diamond_half_triangle.point_c.position = { 0, -1.0f };

			canvas_system.draw_batch_triangle(shape_canvas_resource.batch, diamond_half_triangle);
			shape_canvas_resource.batch_range.count = 2;
		}
		break;
		}
	}

	View::CanvasResource& View::add_sprite_resource(SpriteResourceHandle sprite_handle)
	{
		CanvasResource& canvas_resource = add_canvas_resource(ViewResourceHandle::from_resource_handle(sprite_handle));
		update_sprite_resource(sprite_handle, canvas_resource);

		return canvas_resource;
	}

	void View::update_sprite_resource(SpriteResourceHandle sprite_handle, CanvasResource& sprite_canvas_resource)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const Sprite* sprite = resource_system.get_resource(sprite_handle);

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_batch(sprite_canvas_resource.batch);

		sprite_canvas_resource.batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(sprite_canvas_resource.batch));
		if (sprite->texture_srv.is_valid() == false)
		{
			// Nothing to draw
			sprite_canvas_resource.batch_range.count = 0;
			return;
		}

		const int tile_dim = std::max(sprite->repeat, 1);

		sprite_canvas_resource.batch_range.count = tile_dim * tile_dim;

		if (sprite_canvas_resource.batch_range.count > 1)
		{
			Vadon::Render::Canvas::Sprite current_tile_sprite;
			current_tile_sprite.dimensions.size = Vadon::Utilities::Vector2_One / static_cast<float>(tile_dim);
			current_tile_sprite.texture_view_handle = sprite->texture_srv;

			const float tile_pos_offset = (0.5f / static_cast<float>(tile_dim));

			for (int current_x = 0; current_x < tile_dim; ++current_x)
			{
				for (int current_y = 0; current_y < tile_dim; ++current_y)
				{
					current_tile_sprite.dimensions.position.x = (static_cast<float>(current_x) / static_cast<float>(tile_dim)) - 0.5f + tile_pos_offset;
					current_tile_sprite.dimensions.position.y = (static_cast<float>(current_y) / static_cast<float>(tile_dim)) - 0.5f + tile_pos_offset;

					const int rotation = m_texture_dist(m_random_engine);
					switch (rotation)
					{
					case 1:
						current_tile_sprite.uv_top_left = { 0, 1 };
						current_tile_sprite.uv_top_right = Vadon::Utilities::Vector2_Zero;
						current_tile_sprite.uv_bottom_left = Vadon::Utilities::Vector2_One;
						current_tile_sprite.uv_bottom_right = { 1, 0 };
						break;
					case 2:
						current_tile_sprite.uv_top_left = Vadon::Utilities::Vector2_One;
						current_tile_sprite.uv_top_right = { 0, 1 };
						current_tile_sprite.uv_bottom_left = { 1, 0 };
						current_tile_sprite.uv_bottom_right = Vadon::Utilities::Vector2_Zero;
						break;
					case 3:
						current_tile_sprite.uv_top_left = { 1, 0 };
						current_tile_sprite.uv_top_right = Vadon::Utilities::Vector2_One;
						current_tile_sprite.uv_bottom_left = Vadon::Utilities::Vector2_Zero;
						current_tile_sprite.uv_bottom_right = { 0, 1 };
						break;
					}

					canvas_system.draw_batch_sprite(sprite_canvas_resource.batch, current_tile_sprite);
				}
			}
		}
		else
		{
			Vadon::Render::Canvas::Sprite canvas_sprite;
			canvas_sprite.dimensions.size = Vadon::Utilities::Vector2_One;
			canvas_sprite.uv_top_left = Vadon::Utilities::Vector2_Zero;
			canvas_sprite.uv_bottom_right = Vadon::Utilities::Vector2_One;
			canvas_sprite.texture_view_handle = sprite->texture_srv;

			canvas_system.draw_batch_sprite(sprite_canvas_resource.batch, canvas_sprite);
		}
	}

	void View::update_entity_transform(const Model::Transform2D* transform, ViewComponent* view_component, Render::CanvasComponent* canvas_component)
	{
		VADON_ASSERT(view_component != nullptr, "No view component!");

		Vadon::Render::Canvas::Transform new_transform;
		if (transform != nullptr)
		{
			new_transform.position = transform->position;
			new_transform.scale = transform->scale;
		}

		// Set both previous and current transform to be the same (to prevent "snapping")
		view_component->prev_transform = new_transform;
		view_component->current_transform = new_transform;

		if ((canvas_component == nullptr) || (canvas_component->canvas_item.is_valid() == false))
		{
			// Canvas item not yet initialized
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.set_item_transform(canvas_component->canvas_item, new_transform);
	}

	void View::update_entity_draw_data(const Model::Transform2D* transform, ViewComponent* view_component, Render::CanvasComponent* canvas_component)
	{
		if (canvas_component == nullptr)
		{
			return;
		}

		if (canvas_component->canvas_item.is_valid() == false)
		{
			// Nothing to update
			return;
		}

		update_entity_transform(transform, view_component, canvas_component);

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_item(canvas_component->canvas_item);

		if (view_component->resource.is_valid() == false)
		{
			return;
		}
		
		CanvasResource& resource = get_resource(view_component->resource);
		canvas_system.add_item_batch_draw(canvas_component->canvas_item,
			Vadon::Render::Canvas::BatchDrawCommand
			{
				.batch = resource.batch,
				.range = resource.batch_range
			}
		);
	}
}