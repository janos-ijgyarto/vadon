#include <VadonDemo/View/View.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/View/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <unordered_map>

namespace
{
	struct CanvasResource
	{
		Vadon::Render::Canvas::BatchHandle batch; // Batch of canvas commands corresponding to this resource
		Vadon::Utilities::DataRange batch_range;
	};
}

namespace VadonDemo::View
{
	struct View::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;

		std::unordered_map<uint64_t, CanvasResource> m_canvas_resource_lookup;

		Internal(Vadon::Core::EngineCoreInterface& core)
			: m_engine_core(core)
		{ }

		bool initialize()
		{
			ViewResource::register_resource();
			Shape::register_resource();
			Sprite::register_resource();

			ViewComponent::register_component();

			return true;
		}

		bool init_visualization(Vadon::ECS::World& /*ecs_world*/)
		{
			// TODO: anything?
			return true;
		}

		void extract_model_state(Vadon::ECS::World& ecs_world)
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

		void lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			const float neg_lerp_factor = 1.0f - lerp_factor;

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto view_query = component_manager.run_component_query<ViewComponent&>();
			for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
			{
				auto view_tuple = view_it.get_tuple();
				ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

				if (current_view_component.canvas_item.is_valid() == false)
				{
					// FIXME: should we get this system to add the Canvas Item?
					continue;
				}

				// Update Item with interpolated transform
				Vadon::Render::Canvas::Transform interpolated_transform;
				interpolated_transform.position = current_view_component.current_transform.position * lerp_factor + current_view_component.prev_transform.position * neg_lerp_factor;
				interpolated_transform.scale = current_view_component.current_transform.scale;

				canvas_system.set_item_transform(current_view_component.canvas_item, interpolated_transform);
			}
		}

		CanvasResource* find_view_resource(ViewResourceHandle view_resource_handle)
		{
			auto resource_it = m_canvas_resource_lookup.find(view_resource_handle.to_uint());
			if (resource_it != m_canvas_resource_lookup.end())
			{
				return &resource_it->second;
			}

			return nullptr;
		}

		CanvasResource& get_view_resource(ViewResourceHandle view_resource_handle)
		{
			CanvasResource* resource = find_view_resource(view_resource_handle);
			if (resource != nullptr)
			{
				return *resource;
			}

			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();			
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

		CanvasResource& add_canvas_resource(ViewResourceHandle view_resource_handle)
		{
			CanvasResource& canvas_resource = m_canvas_resource_lookup[view_resource_handle.to_uint()];

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_resource.batch = canvas_system.create_batch();
			
			return canvas_resource;
		}

		CanvasResource& add_shape_resource(ShapeResourceHandle shape_handle)
		{
			CanvasResource& canvas_resource = add_canvas_resource(ViewResourceHandle::from_resource_handle(shape_handle));
			update_shape_resource(shape_handle, canvas_resource);

			return canvas_resource;
		}

		void update_shape_resource(ShapeResourceHandle shape_handle, CanvasResource& shape_canvas_resource)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Shape* shape = resource_system.get_resource(shape_handle);

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

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

		CanvasResource& add_sprite_resource(SpriteResourceHandle /*sprite_handle*/)
		{
			// TODO!!!
			VADON_UNREACHABLE;
		}

		void update_sprite_resource(SpriteResourceHandle /*sprite_handle*/)
		{
			// TODO!!!
			VADON_UNREACHABLE;
		}

		void update_view_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
		{
			auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Model::Transform2D, ViewComponent>(view_entity);

			ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);
			if (view_component == nullptr)
			{
				return;
			}
			const Model::Transform2D* transform = std::get<Model::Transform2D*>(component_tuple);
			update_view_entity_transform(transform, view_component);
		}

		void update_view_entity_transform(const Model::Transform2D* transform, ViewComponent* view_component)
		{
			VADON_ASSERT(view_component != nullptr, "No view component!");

			if (view_component->canvas_item.is_valid() == false)
			{
				// Nothing to update
				return;
			}

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			Vadon::Render::Canvas::Transform new_transform;
			if (transform != nullptr)
			{
				new_transform.position = transform->position;
				new_transform.scale = transform->scale;
			}

			// Set both previous and current transform to be the same (to prevent "snapping")
			view_component->prev_transform = new_transform;
			view_component->current_transform = new_transform;

			canvas_system.set_item_transform(view_component->canvas_item, new_transform);
		}

		void update_view_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
		{
			auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Model::Transform2D, ViewComponent>(view_entity);

			ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);

			if (view_component == nullptr)
			{
				return;
			}

			const Model::Transform2D* transform = std::get<Model::Transform2D*>(component_tuple);
			update_view_entity_draw_data(transform, view_component);
		}

		void update_view_entity_draw_data(const Model::Transform2D* transform, ViewComponent* view_component)
		{
			if (view_component->canvas_item.is_valid() == false)
			{
				// Nothing to update
				return;
			}

			update_view_entity_transform(transform, view_component);

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.set_item_z_order(view_component->canvas_item, view_component->z_order);

			if (view_component->resource.is_valid() == true)
			{
				canvas_system.clear_item(view_component->canvas_item);

				CanvasResource& resource = get_view_resource(view_component->resource);
				canvas_system.add_item_batch_draw(view_component->canvas_item,
					Vadon::Render::Canvas::BatchDrawCommand
					{
						.batch = resource.batch,
						.range = resource.batch_range
					}
				);
			}
		}

		void update_view_resource(Vadon::ECS::World& ecs_world, ViewResourceHandle resource_handle)
		{
			CanvasResource* resource = find_view_resource(resource_handle);
			if (resource != nullptr)
			{
				Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
				const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
				if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
				{
					update_shape_resource(ShapeResourceHandle::from_resource_handle(resource_handle), *resource);
				}
				else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
				{
					update_sprite_resource(SpriteResourceHandle::from_resource_handle(resource_handle));
				}
			}
			else
			{
				resource = &get_view_resource(resource_handle);
			}

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			// FIXME: could do this deferred so we only have to iterate over the entities once
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto view_query = component_manager.run_component_query<ViewComponent&>();
			for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
			{
				auto view_tuple = view_it.get_tuple();
				ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

				if (current_view_component.resource != resource_handle)
				{
					continue;
				}

				canvas_system.clear_item(current_view_component.canvas_item);
				canvas_system.add_item_batch_draw(current_view_component.canvas_item,
					Vadon::Render::Canvas::BatchDrawCommand
					{
						.batch = resource->batch,
						.range = resource->batch_range
					}
				);
			}
		}
	};

	View::View(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(engine_core))
	{
	}

	View::~View()
	{
	}

	bool View::initialize()
	{
		return m_internal->initialize();
	}

	bool View::init_visualization(Vadon::ECS::World& ecs_world)
	{
		return m_internal->init_visualization(ecs_world);
	}

	void View::extract_model_state(Vadon::ECS::World& ecs_world)
	{
		m_internal->extract_model_state(ecs_world);
	}

	void View::lerp_view_state(Vadon::ECS::World& ecs_world, float lerp_factor)
	{
		m_internal->lerp_view_state(ecs_world, lerp_factor);
	}

	void View::update_view_entity_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		m_internal->update_view_entity_transform(ecs_world, view_entity);
	}

	void View::update_view_entity_draw_data(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle view_entity)
	{
		m_internal->update_view_entity_draw_data(ecs_world, view_entity);
	}

	void View::update_view_resource(Vadon::ECS::World& ecs_world, ViewResourceHandle resource_handle)
	{
		m_internal->update_view_resource(ecs_world, resource_handle);
	}
}