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
		Vadon::Render::Canvas::ItemHandle item; // Item used for rendering
		Vadon::Render::Canvas::BatchHandle batch; // Batch of canvas commands corresponding to this object
		Vadon::Utilities::DataRange batch_range;
	};
}

namespace VadonDemo::View
{
	struct View::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;

		Vadon::Render::Canvas::LayerHandle m_canvas_layer;

		std::unordered_map<uint64_t, CanvasResource> m_canvas_resource_lookup;

		std::vector<Vadon::ECS::EntityHandle> m_uninitialized_entities;

		Internal(Vadon::Core::EngineCoreInterface& core)
			: m_engine_core(core)
		{ }

		bool initialize()
		{
			ViewResource::register_resource();
			Shape::register_resource();
			Sprite::register_resource();

			ViewComponent::register_component();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			m_canvas_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

			if (m_canvas_layer.is_valid() == false)
			{
				return false;
			}

			return true;
		}

		bool init_visualization(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			component_manager.register_event_callback<ViewComponent>(
				[this](const Vadon::ECS::ComponentEvent& event)
				{	
					if (event.event_type == Vadon::ECS::ComponentEventType::ADDED)
					{
						m_uninitialized_entities.push_back(event.owner);
					}
				}
			);

			// TODO: pre-register view resources
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

		void update(Vadon::ECS::World& ecs_world, float lerp_factor)
		{
			if (m_uninitialized_entities.empty() == false)
			{
				for (Vadon::ECS::EntityHandle current_entity : m_uninitialized_entities)
				{
					auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Model::Transform2D, ViewComponent>(current_entity);
					const Model::Transform2D* transform = std::get<Model::Transform2D*>(component_tuple);
					ViewComponent* view_component = std::get<ViewComponent*>(component_tuple);

					if ((transform != nullptr) && (view_component != nullptr))
					{
						view_component->prev_transform.position = transform->position;
						view_component->prev_transform.scale = transform->scale;

						view_component->current_transform.position = transform->position;
						view_component->current_transform.scale = transform->scale;
					}
				}

				m_uninitialized_entities.clear();
			}

			lerp_state(ecs_world, lerp_factor);
			// TODO: anything else?
		}

		void lerp_state(Vadon::ECS::World& ecs_world, float lerp_factor)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			const float neg_lerp_factor = 1.0f - lerp_factor;

			// Clear the previous canvas item commands
			for (const auto& canvas_resource_pair : m_canvas_resource_lookup)
			{
				const CanvasResource& current_resource = canvas_resource_pair.second;
				canvas_system.clear_item(current_resource.item);
			}

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto view_query = component_manager.run_component_query<ViewComponent&>();
			for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
			{
				auto view_tuple = view_it.get_tuple();
				ViewComponent& current_view_component = std::get<ViewComponent&>(view_tuple);

				if (current_view_component.resource.is_valid() == false)
				{
					continue;
				}

				auto resource_it = m_canvas_resource_lookup.find(current_view_component.resource.to_uint());
				if (resource_it == m_canvas_resource_lookup.end())
				{
					add_view_resource(current_view_component.resource);
					resource_it = m_canvas_resource_lookup.find(current_view_component.resource.to_uint());
				}
				
				// Add batch draw with the interpolated transform
				Vadon::Render::Canvas::Transform interpolated_transform;
				interpolated_transform.position = current_view_component.current_transform.position * lerp_factor + current_view_component.prev_transform.position * neg_lerp_factor;
				interpolated_transform.scale = current_view_component.current_transform.scale;

				const CanvasResource& canvas_resource = resource_it->second;
				Vadon::Render::Canvas::BatchDrawCommand batch_command =
				{
					.batch = canvas_resource.batch,
					.transform = interpolated_transform,
					.range = canvas_resource.batch_range
				};

				canvas_system.add_item_batch_draw(canvas_resource.item, batch_command);
			}
		}

		void add_view_resource(ViewResourceHandle view_resource_handle)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();			
			const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(view_resource_handle);
			if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
			{
				add_shape_resource(ShapeResourceHandle::from_resource_handle(view_resource_handle));
			}
			else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
			{
				add_sprite_resource(SpriteResourceHandle::from_resource_handle(view_resource_handle));
			}
		}

		void add_canvas_resource(ViewResourceHandle view_resource_handle)
		{
			CanvasResource& canvas_resource = m_canvas_resource_lookup[view_resource_handle.to_uint()];

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_resource.batch = canvas_system.create_batch();
			canvas_resource.item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = m_canvas_layer });
		}

		void add_shape_resource(ShapeResourceHandle shape_handle)
		{
			add_canvas_resource(ViewResourceHandle::from_resource_handle(shape_handle));
			update_shape_resource(shape_handle);
		}

		void update_shape_resource(ShapeResourceHandle shape_handle)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Shape* shape = resource_system.get_resource(shape_handle);

			CanvasResource& shape_canvas_resource = m_canvas_resource_lookup[shape_handle.to_uint()];
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

			canvas_system.set_item_z_order(shape_canvas_resource.item, shape->z_order);
		}

		void add_sprite_resource(SpriteResourceHandle /*sprite_handle*/)
		{
			// TODO!!!
			assert(false);
		}

		void update_sprite_resource(SpriteResourceHandle /*sprite_handle*/)
		{
			// TODO!!!
			assert(false);
		}

		void update_view_resource(ViewResourceHandle resource_handle)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
			if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Shape>())
			{
				update_shape_resource(ShapeResourceHandle::from_resource_handle(resource_handle));
			}
			else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
			{
				update_sprite_resource(SpriteResourceHandle::from_resource_handle(resource_handle));
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

	void View::update(Vadon::ECS::World& ecs_world, float lerp_factor)
	{
		m_internal->update(ecs_world, lerp_factor);
	}

	Vadon::Render::Canvas::LayerHandle View::get_canvas_layer() const
	{
		return m_internal->m_canvas_layer;
	}

	void View::update_view_resource(ViewResourceHandle resource_handle)
	{
		m_internal->update_view_resource(resource_handle);
	}
}