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

			if (model_transform.teleported == false)
			{
				current_view_component.prev_transform = current_view_component.current_transform;

				// TODO: rotation!
				current_view_component.current_transform.position = model_transform.position;
				current_view_component.current_transform.scale = model_transform.scale;
			}
			else
			{
				// TODO: rotation!
				current_view_component.current_transform.position = model_transform.position;
				current_view_component.current_transform.scale = model_transform.scale;

				current_view_component.prev_transform = current_view_component.current_transform;
			}
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

	ViewResourceHandle View::load_view_resource(ViewResourceID resource_id) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		return resource_system.load_resource(resource_id);
	}

	void View::load_resource_data(ViewResourceID resource_id)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const ViewResourceHandle resource_handle = load_view_resource(resource_id);
		ViewResource* view_resource = resource_system.get_resource(resource_handle);

		if (view_resource->batch.is_valid() == false)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			view_resource->batch = canvas_system.create_batch();
			view_resource->batch_range = Vadon::Utilities::DataRange();
		}

		if (view_resource->batch_range.is_valid() == true)
		{
			// Resource already loaded
			return;
		}

		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
		{
			load_shape_resource(static_cast<Shape*>(view_resource));
		}
		else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
		{
			load_sprite_resource(static_cast<Sprite*>(view_resource));
		}
		else
		{
			VADON_UNREACHABLE;
		}
	}

	void View::reset_resource_data(ViewResourceID resource_id)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const ViewResourceHandle resource_handle = load_view_resource(resource_id);
		ViewResource* view_resource = resource_system.get_resource(resource_handle);
		if (view_resource->batch_range.is_valid() == false)
		{
			// Nothing to do
			return;
		}

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_batch(view_resource->batch);

		view_resource->batch_range = Vadon::Utilities::DataRange();
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

	void View::load_shape_resource(Shape* shape)
	{
		VADON_ASSERT(shape->batch_range.is_valid() == false, "Shape already loaded!");

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		shape->batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(shape->batch));

		switch (Vadon::Utilities::to_enum<ShapeType>(shape->type))
		{
		case ShapeType::TRIANGLE:
		{
			Vadon::Render::Canvas::Triangle triangle;
			triangle.points[0].position = {0, 0.5f};
			triangle.points[0].color = shape->color;

			triangle.points[1].position = { 0.5f, -0.5f };
			triangle.points[1].color = shape->color;

			triangle.points[2].position = { -0.5f, -0.5f };
			triangle.points[2].color = shape->color;

			canvas_system.draw_batch_triangle(shape->batch, triangle);
			shape->batch_range.count = 1;
		}
		break;
		case ShapeType::RECTANGLE:
		{
			Vadon::Render::Canvas::Rectangle box_rectangle;
			box_rectangle.color = shape->color;
			box_rectangle.dimensions.size = { 1.0f, 1.0f };
			box_rectangle.filled = true;

			canvas_system.draw_batch_rectangle(shape->batch, box_rectangle);
			shape->batch_range.count = 1;
		}
		break;
		case ShapeType::DIAMOND:
		{
			Vadon::Render::Canvas::Triangle diamond_half_triangle;
			diamond_half_triangle.points[0].position = { 0, 1.0f };
			diamond_half_triangle.points[1].position = { 1.0f, 0 };
			diamond_half_triangle.points[2].position = { -1.0f, 0 };
			
			for (size_t triangle_index = 0; triangle_index < 3; ++triangle_index)
			{
				diamond_half_triangle.points[triangle_index].color = shape->color;
			}

			canvas_system.draw_batch_triangle(shape->batch, diamond_half_triangle);

			diamond_half_triangle.points[0].position = { -1.0f, 0 };
			diamond_half_triangle.points[1].position = { 1.0f, 0 };
			diamond_half_triangle.points[2].position = { 0, -1.0f };

			canvas_system.draw_batch_triangle(shape->batch, diamond_half_triangle);
			shape->batch_range.count = 2;
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

		sprite->batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(sprite->batch));
		sprite->batch_range.count = 2;

		Vadon::Render::Canvas::Sprite canvas_sprite;
		canvas_sprite.dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.uv_dimensions.size = Vadon::Math::Vector2_One;
		canvas_sprite.color = Vadon::Math::Color_White;

		canvas_system.set_batch_texture(sprite->batch, Vadon::Render::Canvas::Texture{ .srv = sprite_texture->texture_srv });
		canvas_system.draw_batch_sprite(sprite->batch, canvas_sprite);
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

		// Clear the canvas item (we shouldn't show anything if we have no valid data)
		// TODO: show some placeholder instead?
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_item(canvas_component->canvas_item);

		if (view_component->resource.is_valid() == false)
		{
			return;
		}

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		ViewResourceHandle view_resource_handle = ViewResourceHandle::from_resource_handle(resource_system.load_resource(view_component->resource));
		const ViewResource* view_resource = resource_system.get_resource(view_resource_handle);
		
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
}