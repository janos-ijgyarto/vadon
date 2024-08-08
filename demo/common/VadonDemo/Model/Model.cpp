#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

namespace VadonDemo::Model
{
	namespace
	{
		void draw_canvas_item(Vadon::Render::Canvas::CanvasSystem& canvas_system, const Transform2D* transform, CanvasComponent& canvas_item, const Celestial* celestial)
		{
			canvas_system.clear_item(canvas_item.canvas_handle);

			const float scale = (transform != nullptr) ? transform->scale : 1.0f;

			if (celestial != nullptr)
			{
				if (celestial->is_star == true)
				{
					Vadon::Render::Canvas::Triangle star_triangle;
					star_triangle.color = Vadon::Render::Canvas::ColorRGBA(canvas_item.color, 1.0f);

					star_triangle.point_a.position = { 0, scale };
					star_triangle.point_b.position = { scale, 0 };
					star_triangle.point_c.position = { -scale, 0 };

					canvas_system.draw_triangle(canvas_item.canvas_handle, star_triangle);

					star_triangle.point_a.position = { -scale, 0 };
					star_triangle.point_b.position = { scale, 0 };
					star_triangle.point_c.position = { 0, -scale };

					canvas_system.draw_triangle(canvas_item.canvas_handle, star_triangle);
				}
				else
				{
					Vadon::Render::Canvas::Rectangle planet_rectangle;
					planet_rectangle.color = Vadon::Render::Canvas::ColorRGBA(canvas_item.color, 1.0f);
					planet_rectangle.dimensions.size = { scale, scale };
					planet_rectangle.filled = true;

					canvas_system.draw_rectangle(canvas_item.canvas_handle, planet_rectangle);
				}
			}
			else
			{
				Vadon::Render::Canvas::Triangle placeholder_triangle;
				placeholder_triangle.color = Vadon::Render::Canvas::ColorRGBA(canvas_item.color, 1.0f);

				placeholder_triangle.point_a.position = { 0, scale * 0.5f };
				placeholder_triangle.point_b.position = { scale * 0.5f, - scale * 0.5f };
				placeholder_triangle.point_c.position = { -scale * 0.5f, -scale * 0.5f };

				canvas_system.draw_triangle(canvas_item.canvas_handle, placeholder_triangle);
			}

			canvas_item.scale = scale;
		}

		Vadon::Utilities::Vector2 rotate_2d_vector(const Vadon::Utilities::Vector2& vector, float angle)
		{
			const float sin_angle = std::sinf(angle);
			const float cos_angle = std::cosf(angle);

			return Vadon::Utilities::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
		}
	}

	struct Model::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Vadon::Render::Canvas::LayerHandle m_canvas_layer; // FIXME: should not embed this in model!

		Internal(Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{
		}

		bool initialize()
		{
			Transform2D::register_component();
			CanvasComponent::register_component();
			Celestial::register_component();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			m_canvas_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

			return true;
		}

		void init_simulation(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			// TODO: query those who don't have transform and warn about them?
			auto model_query = component_manager.run_component_query<Transform2D&, Celestial&>();
			auto model_it = model_query.get_iterator();

			for (; model_it.is_valid() == true; model_it.next())
			{
				auto current_entity = model_it.get_entity();
				auto current_component_tuple = model_it.get_tuple();

				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);
				Celestial& celestial_component = std::get<Celestial&>(current_component_tuple);

				// Radius will be equal to the offset via local transform
				celestial_component.radius = Vadon::Utilities::length(transform_component.position);
			}
		}

		void update_simulation(Vadon::ECS::World& ecs_world, float delta_time)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto model_query = component_manager.run_component_query<Transform2D&, Celestial&>();
			auto model_it = model_query.get_iterator();

			for (; model_it.is_valid() == true; model_it.next())
			{
				auto current_entity = model_it.get_entity();
				auto current_component_tuple = model_it.get_tuple();

				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);
				Celestial& celestial_component = std::get<Celestial&>(current_component_tuple);

				// If radius or angular velocity too small, we skip update
				if ((Vadon::Utilities::abs(celestial_component.angular_velocity) < 0.001f) || (Vadon::Utilities::dot(transform_component.position, transform_component.position) < 0.001f))
				{
					continue;
				}
				
				const Vadon::Utilities::Vector2 norm_vector = Vadon::Utilities::normalize(transform_component.position);
				const Vadon::Utilities::Vector2 rotated_vector = Vadon::Utilities::normalize(rotate_2d_vector(norm_vector, delta_time * celestial_component.angular_velocity));

				transform_component.position = rotated_vector * celestial_component.radius;
			}
		}

		void update_rendering(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			auto render_query = component_manager.run_component_query<CanvasComponent*, Transform2D&, Celestial*>();
			auto render_it = render_query.get_iterator();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();
				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);

				{
					// Update transform global position
					// FIXME: have a system which handles this more efficiently!
					auto parent_entity = entity_manager.get_entity_parent(current_entity);
					if (parent_entity.is_valid() == true)
					{
						Transform2D* parent_transform = component_manager.get_component<Transform2D>(parent_entity);
						if (parent_transform != nullptr)
						{
							transform_component.global_position = parent_transform->global_position + transform_component.position;
						}
						else
						{
							transform_component.global_position = transform_component.position;
						}
					}
				}

				CanvasComponent* canvas_item = std::get<CanvasComponent*>(current_component_tuple);
				if (canvas_item != nullptr)
				{
					if (canvas_item->canvas_handle.is_valid() == true)
					{
						// Update position
						canvas_system.set_item_position(canvas_item->canvas_handle, transform_component.global_position);
						Celestial* celestial = std::get<Celestial*>(current_component_tuple);
						if (canvas_item->scale != transform_component.scale)
						{
							// Scale updated, redraw
							// FIXME: should be able to scale independently!
							draw_canvas_item(canvas_system, &transform_component, *canvas_item, celestial);
						}
					}
				}
			}
		}

		void component_event(Vadon::ECS::World& ecs_world, const Vadon::ECS::ComponentEvent& event)
		{
			// FIXME: this is just to get it working
			// Ideally client code will define model and view entities as needed
			// e.g model entities will spawn editor-specific entities to use for visualization, drag & drop, etc.
			switch (event.event_type)
			{
			case Vadon::ECS::ComponentEventType::ADDED:
			{
				if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<CanvasComponent>())
				{
					// Create canvas item
					auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Transform2D, CanvasComponent, Celestial>(event.owner);
					CanvasComponent* canvas_component = std::get<CanvasComponent*>(component_tuple);

					Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

					Vadon::Render::Canvas::ItemInfo new_item_info;
					new_item_info.layer = m_canvas_layer;

					canvas_component->canvas_handle = canvas_system.create_item(new_item_info);

					draw_canvas_item(canvas_system, std::get<Transform2D*>(component_tuple), *canvas_component, std::get<Celestial*>(component_tuple));
				}
				else if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<Celestial>())
				{
					// Redraw canvas item (if present)
					auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Transform2D, CanvasComponent, Celestial>(event.owner);
					CanvasComponent* canvas_component = std::get<CanvasComponent*>(component_tuple);
					if (canvas_component != nullptr)
					{
						Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
						draw_canvas_item(canvas_system, std::get<Transform2D*>(component_tuple), *canvas_component, std::get<Celestial*>(component_tuple));
					}
				}
				break;
			}
			case Vadon::ECS::ComponentEventType::REMOVED:
			{
				if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<CanvasComponent>())
				{
					// Remove canvas item
					CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(event.owner);
					if (canvas_component->canvas_handle.is_valid() == true)
					{
						Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
						canvas_system.remove_item(canvas_component->canvas_handle);
					}
				}
				else if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<Celestial>())
				{
					// Redraw canvas item (if present)
					auto component_tuple = ecs_world.get_component_manager().get_component_tuple<Transform2D, CanvasComponent, Celestial>(event.owner);
					CanvasComponent* canvas_component = std::get<CanvasComponent*>(component_tuple);
					if (canvas_component != nullptr)
					{
						Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
						draw_canvas_item(canvas_system, std::get<Transform2D*>(component_tuple), *canvas_component, std::get<Celestial*>(component_tuple));
					}
				}
				break;
			}
			}
		}
	};

	Model::Model(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(engine_core))
	{
	}

	Model::~Model() = default;

	bool Model::initialize()
	{
		return m_internal->initialize();
	}
	
	void Model::init_simulation(Vadon::ECS::World& ecs_world)
	{
		m_internal->init_simulation(ecs_world);
	}

	void Model::update_simulation(Vadon::ECS::World& ecs_world, float delta_time)
	{
		m_internal->update_simulation(ecs_world, delta_time);
	}

	void Model::update_rendering(Vadon::ECS::World& ecs_world)
	{
		m_internal->update_rendering(ecs_world);
	}

	Vadon::Render::Canvas::LayerHandle Model::get_canvas_layer() const
	{
		return m_internal->m_canvas_layer;
	}

	VADONDEMO_API void Model::component_event(Vadon::ECS::World& ecs_world, const Vadon::ECS::ComponentEvent& event)
	{
		m_internal->component_event(ecs_world, event);
	}

	VADONDEMO_API void Model::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);

		// TODO: register types as needed!
	}
}