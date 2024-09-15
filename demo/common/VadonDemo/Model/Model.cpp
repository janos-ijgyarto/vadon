#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

namespace VadonDemo::Model
{
	namespace
	{
		struct CanvasRenderObject
		{
			// FIXME: split up these into "components" as well?
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;
			float scale = 1.0f;

			Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
			float object_scale = 1.0f;

			RenderObjectType type = RenderObjectType::TRIANGLE;

			Vadon::Render::Canvas::ItemHandle canvas_handle;
		};

		struct RenderObjectCreate
		{
			int32_t handle;
			// TODO: anything else?
		};

		struct RenderObjectDestroy
		{
			int32_t handle;
			// TODO: anything else?
		};

		struct RenderTransformUpdate
		{
			int32_t handle;
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;
			float scale = 1.0f;
		};

		struct RenderMaterialUpdate
		{
			int32_t handle;
			Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
		};

		struct RenderTypeUpdate
		{
			int32_t handle;
			RenderObjectType type = RenderObjectType::TRIANGLE;
		};

		void draw_canvas_item(Vadon::Render::Canvas::CanvasSystem& canvas_system, const CanvasRenderObject& render_object)
		{
			if (render_object.canvas_handle.is_valid() == false)
			{
				return;
			}

			canvas_system.clear_item(render_object.canvas_handle);

			switch (render_object.type)
			{
			case RenderObjectType::TRIANGLE:
			{
				Vadon::Render::Canvas::Triangle triangle;
				triangle.color = Vadon::Render::Canvas::ColorRGBA(render_object.color, 1.0f);

				triangle.point_a.position = { 0, 0.5f };
				triangle.point_b.position = { 0.5f, -0.5f };
				triangle.point_c.position = { -0.5f, -0.5f };

				canvas_system.draw_triangle(render_object.canvas_handle, triangle);
			}
			break;
			case RenderObjectType::BOX:
			{
				Vadon::Render::Canvas::Rectangle planet_rectangle;
				planet_rectangle.color = Vadon::Render::Canvas::ColorRGBA(render_object.color, 1.0f);
				planet_rectangle.dimensions.size = { 1.0f, 1.0f };
				planet_rectangle.filled = true;

				canvas_system.draw_rectangle(render_object.canvas_handle, planet_rectangle);
			}
			break;
			case RenderObjectType::DIAMOND:
			{
				Vadon::Render::Canvas::Triangle diamond_half_triangle;
				diamond_half_triangle.color = Vadon::Render::Canvas::ColorRGBA(render_object.color, 1.0f);

				diamond_half_triangle.point_a.position = { 0, 1.0f };
				diamond_half_triangle.point_b.position = { 1.0f, 0 };
				diamond_half_triangle.point_c.position = { -1.0f, 0 };

				canvas_system.draw_triangle(render_object.canvas_handle, diamond_half_triangle);

				diamond_half_triangle.point_a.position = { -1.0f, 0 };
				diamond_half_triangle.point_b.position = { 1.0f, 0 };
				diamond_half_triangle.point_c.position = { 0, -1.0f };

				canvas_system.draw_triangle(render_object.canvas_handle, diamond_half_triangle);
			}
			break;
			}
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

		// FIXME: implement a proper object pool, just doing this to keep it simple for now
		std::vector<CanvasRenderObject> m_render_object_pool;

		int32_t m_active_object_count = 0;
		std::vector<int32_t> m_render_free_list;

		// Required because some events will have been enqueued outside of the frame update
		Vadon::Utilities::PacketQueue m_render_event_queue;

		float m_interpolation_weight = 1.0f;

		Internal(Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{
		}

		bool initialize()
		{
			Transform2D::register_component();
			CanvasComponent::register_component();
			Celestial::register_component();

			Vadon::Utilities::TypeRegistry::register_type<RenderObjectCreate>();
			Vadon::Utilities::TypeRegistry::register_type<RenderObjectDestroy>();
			Vadon::Utilities::TypeRegistry::register_type<RenderTransformUpdate>();
			Vadon::Utilities::TypeRegistry::register_type<RenderMaterialUpdate>();
			Vadon::Utilities::TypeRegistry::register_type<RenderTypeUpdate>();

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

		void update_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity_handle, Transform2D& transform_component)
		{
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			transform_component.prev_position = transform_component.global_position;

			// FIXME: have a system which handles this more efficiently!
			auto parent_entity = entity_manager.get_entity_parent(entity_handle);
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
			else
			{
				transform_component.global_position = transform_component.position;
			}
		}

		void update_simulation(Vadon::ECS::World& ecs_world, float delta_time)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto model_query = component_manager.run_component_query<Transform2D&, Celestial*>();
			auto model_it = model_query.get_iterator();

			for (; model_it.is_valid() == true; model_it.next())
			{
				auto current_entity = model_it.get_entity();
				auto current_component_tuple = model_it.get_tuple();

				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);
				Celestial* celestial_component = std::get<Celestial*>(current_component_tuple);

				update_transform(ecs_world, current_entity, transform_component);

				if (celestial_component != nullptr)
				{
					// If radius or angular velocity too small, we skip update
					if ((Vadon::Utilities::abs(celestial_component->angular_velocity) < 0.001f) || (Vadon::Utilities::dot(transform_component.position, transform_component.position) < 0.001f))
					{
						continue;
					}

					const Vadon::Utilities::Vector2 norm_vector = Vadon::Utilities::normalize(transform_component.position);
					const Vadon::Utilities::Vector2 rotated_vector = Vadon::Utilities::normalize(rotate_2d_vector(norm_vector, delta_time * celestial_component->angular_velocity));

					transform_component.position = rotated_vector * celestial_component->radius;
				}
			}
		}

		void update_view(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue, bool update_transforms)
		{
			render_queue.clear();

			// Prepend with events, if any
			if (m_render_event_queue.is_empty() == false)
			{
				render_queue.append_queue(m_render_event_queue);
				m_render_event_queue.clear();
			}

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto render_query = component_manager.run_component_query<CanvasComponent*, Transform2D&, Celestial*>();
			auto render_it = render_query.get_iterator();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();
				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);

				if (update_transforms == true)
				{
					update_transform(ecs_world, current_entity, transform_component);
				}

				CanvasComponent* canvas_item = std::get<CanvasComponent*>(current_component_tuple);
				if (canvas_item != nullptr)
				{
					if (canvas_item->render_handle < 0)
					{
						if (m_render_free_list.empty() == false)
						{
							canvas_item->render_handle = m_render_free_list.back();
							m_render_free_list.pop_back();
						}
						else
						{
							canvas_item->render_handle = m_active_object_count;
						}
						++m_active_object_count;

						render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectCreate>(), RenderObjectCreate{
							.handle = canvas_item->render_handle
							});

						component_updated(ecs_world, current_entity, Vadon::ECS::ComponentManager::get_component_type_id<CanvasComponent>());
					}

					// Always update position
					// FIXME: allow for objects that are static, i.e don't need constant transform updates!
					render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderTransformUpdate>(), RenderTransformUpdate{
							.handle = canvas_item->render_handle,
							.position = transform_component.global_position,
							.prev_position = transform_component.prev_position,
							.scale = transform_component.scale
						});
				}
			}
		}

		void render_view(const Vadon::Utilities::PacketQueue& render_queue)
		{
			Vadon::Utilities::PacketQueue::Iterator queue_iterator = render_queue.get_iterator();
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			for (; queue_iterator.is_valid() == true; queue_iterator.next())
			{
				const uint32_t header_id = queue_iterator.get_header_id();

				if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectCreate>())
				{
					const RenderObjectCreate* obj_create = reinterpret_cast<const RenderObjectCreate*>(queue_iterator.get_packet_data());

					CanvasRenderObject new_object;					

					Vadon::Render::Canvas::ItemInfo new_item_info;
					new_item_info.layer = m_canvas_layer;

					new_object.canvas_handle = canvas_system.create_item(new_item_info);

					if (obj_create->handle >= m_render_object_pool.size())
					{
						m_render_object_pool.push_back(new_object);
					}
					else
					{
						m_render_object_pool[obj_create->handle] = new_object;
					}

					draw_canvas_item(canvas_system, new_object);
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectDestroy>())
				{
					const RenderObjectDestroy* obj_destroy = reinterpret_cast<const RenderObjectDestroy*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[obj_destroy->handle];
					canvas_system.remove_item(render_object.canvas_handle);
					render_object.canvas_handle.invalidate();
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderTransformUpdate>())
				{
					const RenderTransformUpdate* transform_update = reinterpret_cast<const RenderTransformUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[transform_update->handle];
					render_object.prev_position = render_object.position;
					render_object.position = transform_update->position;
					render_object.scale = transform_update->scale;

					canvas_system.set_item_position(render_object.canvas_handle, render_object.position);
					canvas_system.set_item_scale(render_object.canvas_handle, render_object.scale);
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderMaterialUpdate>())
				{
					const RenderMaterialUpdate* material_update = reinterpret_cast<const RenderMaterialUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[material_update->handle];
					render_object.color = material_update->color;
					
					draw_canvas_item(canvas_system, render_object);
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderTypeUpdate>())
				{
					const RenderTypeUpdate* type_update = reinterpret_cast<const RenderTypeUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[type_update->handle];
					render_object.type = type_update->type;

					draw_canvas_item(canvas_system, render_object);
				}
			}
		}

		void lerp_view_state(float lerp_weight)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			const float neg_lerp_weight = 1.0f - lerp_weight;
			for (const CanvasRenderObject& current_object : m_render_object_pool)
			{
				if (current_object.canvas_handle.is_valid() == false)
				{
					continue;
				}

				const Vadon::Utilities::Vector2 lerp_position = (lerp_weight * current_object.position) + (neg_lerp_weight * current_object.prev_position);
				canvas_system.set_item_position(current_object.canvas_handle, lerp_position);
			}
		}

		void component_event(Vadon::ECS::World& ecs_world, const Vadon::ECS::ComponentEvent& event)
		{
			// FIXME: this is just to get it working
			// Ideally client code will define model and view entities as needed
			// e.g model entities will spawn editor-specific entities to use for visualization, drag & drop, etc.
			switch (event.event_type)
			{
			case Vadon::ECS::ComponentEventType::REMOVED:
			{
				if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<Transform2D>())
				{
					CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(event.owner);
					if (canvas_component != nullptr)
					{
						// Reset the transform
						m_render_event_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderTransformUpdate>(), RenderTransformUpdate{
								.handle = canvas_component->render_handle,
								.position = Vadon::Utilities::Vector2_Zero,
								.scale = 1.0f
							});
					}
				}
				else if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<CanvasComponent>())
				{
					// Remove canvas item
					CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(event.owner);
					if (canvas_component->render_handle >= 0)
					{
						m_render_free_list.push_back(canvas_component->render_handle);
						m_render_event_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectDestroy>(), RenderObjectDestroy{
								.handle = canvas_component->render_handle
							});

						--m_active_object_count;
					}
				}
				break;
			}
			}
		}

		void component_updated(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, Vadon::ECS::ComponentID component)
		{
			if (component == Vadon::ECS::ComponentManager::get_component_type_id<CanvasComponent>())
			{
				CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);

				m_render_event_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderMaterialUpdate>(), RenderMaterialUpdate{
					.handle = canvas_component->render_handle,
					.color = canvas_component->color
					});

				const int32_t type = std::clamp(canvas_component->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));

				m_render_event_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderTypeUpdate>(), RenderTypeUpdate{
					.handle = canvas_component->render_handle,
					.type = Vadon::Utilities::to_enum<RenderObjectType>(type)
					});
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

	void Model::update_view(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue, bool update_transforms)
	{
		m_internal->update_view(ecs_world, render_queue, update_transforms);
	}

	void Model::render_view(const Vadon::Utilities::PacketQueue& render_queue)
	{
		m_internal->render_view(render_queue);
	}

	void Model::lerp_view_state(float lerp_weight)
	{
		m_internal->lerp_view_state(lerp_weight);
	}

	Vadon::Render::Canvas::LayerHandle Model::get_canvas_layer() const
	{
		return m_internal->m_canvas_layer;
	}

	VADONDEMO_API void Model::component_event(Vadon::ECS::World& ecs_world, const Vadon::ECS::ComponentEvent& event)
	{
		m_internal->component_event(ecs_world, event);
	}

	VADONDEMO_API void Model::component_updated(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, Vadon::ECS::ComponentID component)
	{
		m_internal->component_updated(ecs_world, entity, component);
	}

	VADONDEMO_API void Model::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);

		// TODO: register types as needed!
	}
}