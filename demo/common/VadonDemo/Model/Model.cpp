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

		Vadon::Utilities::Vector2 rotate_2d_vector(const Vadon::Utilities::Vector2& vector, float angle)
		{
			const float sin_angle = std::sinf(angle);
			const float cos_angle = std::cosf(angle);

			return Vadon::Utilities::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
		}

		void recursive_update_transform(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, Vadon::Utilities::Vector2 parent_position)
		{
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			Transform2D* transform_component = component_manager.get_component<Transform2D>(entity);
			if (transform_component != nullptr)
			{
				transform_component->global_position = transform_component->position + parent_position;
				for (Vadon::ECS::EntityHandle current_child : entity_manager.get_children(entity))
				{
					recursive_update_transform(ecs_world, current_child, transform_component->global_position);
				}
			}
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

		Vadon::Render::Canvas::BatchHandle m_batch_handle;

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

			m_batch_handle = canvas_system.create_batch();

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

			reset_transforms(ecs_world);
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

		void render_sync(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto render_query = component_manager.run_component_query<CanvasComponent&, Transform2D*>();
			auto render_it = render_query.get_iterator();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();

				Transform2D* transform_component = std::get<Transform2D*>(current_component_tuple);
				CanvasComponent& canvas_component = std::get<CanvasComponent&>(current_component_tuple);

				if (canvas_component.render_handle < 0)
				{
					canvas_component.render_handle = register_canvas_item();

					CanvasRenderObject new_render_object;
					new_render_object.color = canvas_component.color;
					new_render_object.type = Vadon::Utilities::to_enum<RenderObjectType>(canvas_component.type);

					create_canvas_object(canvas_component.render_handle, new_render_object);
					reset_canvas_items();
				}

				CanvasRenderObject& render_object = m_render_object_pool[canvas_component.render_handle];
				if (transform_component != nullptr)
				{
					render_object.position = transform_component->global_position;
					render_object.prev_position = transform_component->prev_position;
					render_object.scale = transform_component->scale;
				}
				else
				{
					render_object.position = Vadon::Utilities::Vector2_Zero;
					render_object.prev_position = Vadon::Utilities::Vector2_Zero;
					render_object.scale = 1.0f;
				}

				canvas_system.set_item_transform(render_object.canvas_handle, Vadon::Render::Canvas::Transform{ .position = render_object.position, .scale = render_object.scale });
			}
		}

		void update_view_async(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue)
		{
			render_queue.clear();

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto render_query = component_manager.run_component_query<CanvasComponent*, Transform2D&>();
			auto render_it = render_query.get_iterator();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();
				Transform2D& transform_component = std::get<Transform2D&>(current_component_tuple);

				CanvasComponent* canvas_component = std::get<CanvasComponent*>(current_component_tuple);
				if (canvas_component != nullptr)
				{
					if (canvas_component->render_handle < 0)
					{
						canvas_component->render_handle = register_canvas_item();

						render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectCreate>(), RenderObjectCreate{
							.handle = canvas_component->render_handle
							});


						render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderMaterialUpdate>(), RenderMaterialUpdate{
							.handle = canvas_component->render_handle,
							.color = canvas_component->color
							});

						const int32_t type = std::clamp(canvas_component->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));

						render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderTypeUpdate>(), RenderTypeUpdate{
							.handle = canvas_component->render_handle,
							.type = Vadon::Utilities::to_enum<RenderObjectType>(type)
							});
					}

					// Always update position
					// FIXME: allow for objects that are static, i.e don't need constant transform updates!
					render_queue.write_packet(Vadon::Utilities::TypeRegistry::get_type_id<RenderTransformUpdate>(), RenderTransformUpdate{
							.handle = canvas_component->render_handle,
							.position = transform_component.global_position,
							.prev_position = transform_component.prev_position,
							.scale = transform_component.scale
						});
				}
			}
		}

		void render_async(const Vadon::Utilities::PacketQueue& render_queue)
		{
			// FIXME: properly decouple view from model!
			Vadon::Utilities::PacketQueue::Iterator queue_iterator = render_queue.get_iterator();
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			for (; queue_iterator.is_valid() == true; queue_iterator.next())
			{
				const uint32_t header_id = queue_iterator.get_header_id();

				if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectCreate>())
				{
					const RenderObjectCreate* obj_create = reinterpret_cast<const RenderObjectCreate*>(queue_iterator.get_packet_data());
					create_canvas_object(obj_create->handle, CanvasRenderObject{});
					reset_canvas_items();
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderObjectDestroy>())
				{
					const RenderObjectDestroy* obj_destroy = reinterpret_cast<const RenderObjectDestroy*>(queue_iterator.get_packet_data());
					internal_remove_canvas_item(m_render_object_pool[obj_destroy->handle]);
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderTransformUpdate>())
				{
					const RenderTransformUpdate* transform_update = reinterpret_cast<const RenderTransformUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[transform_update->handle];
					render_object.prev_position = render_object.position;
					render_object.position = transform_update->position;
					render_object.scale = transform_update->scale;

					canvas_system.set_item_transform(render_object.canvas_handle, Vadon::Render::Canvas::Transform{ .position = render_object.position, .scale = render_object.scale });
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderMaterialUpdate>())
				{
					const RenderMaterialUpdate* material_update = reinterpret_cast<const RenderMaterialUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[material_update->handle];
					render_object.color = material_update->color;
					
					reset_canvas_items();
				}
				else if (header_id == Vadon::Utilities::TypeRegistry::get_type_id<RenderTypeUpdate>())
				{
					const RenderTypeUpdate* type_update = reinterpret_cast<const RenderTypeUpdate*>(queue_iterator.get_packet_data());

					CanvasRenderObject& render_object = m_render_object_pool[type_update->handle];
					render_object.type = type_update->type;

					reset_canvas_items();
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
				canvas_system.set_item_transform(current_object.canvas_handle, Vadon::Render::Canvas::Transform{ .position = lerp_position, .scale = current_object.scale });
			}
		}

		void reset_transforms(Vadon::ECS::World& ecs_world)
		{
			// FIXME: implement a proper system for managing transform hierarchies!
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto transform_query = component_manager.run_component_query<Transform2D&>();
			auto transform_it = transform_query.get_iterator();

			for (; transform_it.is_valid() == true; transform_it.next())
			{
				auto current_entity = transform_it.get_entity();
				const Vadon::ECS::EntityHandle parent_entity = entity_manager.get_entity_parent(current_entity);
				if ((parent_entity.is_valid() == false) || (component_manager.has_component<Transform2D>(parent_entity) == false))
				{
					recursive_update_transform(ecs_world, current_entity, Vadon::Utilities::Vector2_Zero);
				}
			}
		}

		void reset_canvas_items()
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.clear_batch(m_batch_handle);

			Vadon::Render::Canvas::BatchDrawCommand batch_command;
			batch_command.batch = m_batch_handle;

			for (CanvasRenderObject& current_object : m_render_object_pool)
			{
				if (current_object.canvas_handle.is_valid() == false)
				{
					continue;
				}

				canvas_system.clear_item(current_object.canvas_handle);

				batch_command.range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(m_batch_handle));

				switch (current_object.type)
				{
				case RenderObjectType::TRIANGLE:
				{
					Vadon::Render::Canvas::Triangle triangle;
					triangle.color = Vadon::Render::Canvas::ColorRGBA(current_object.color, 1.0f);

					triangle.point_a.position = { 0, 0.5f };
					triangle.point_b.position = { 0.5f, -0.5f };
					triangle.point_c.position = { -0.5f, -0.5f };

					canvas_system.draw_batch_triangle(m_batch_handle, triangle);
					batch_command.range.count = 1;
				}
				break;
				case RenderObjectType::BOX:
				{
					Vadon::Render::Canvas::Rectangle box_rectangle;
					box_rectangle.color = Vadon::Render::Canvas::ColorRGBA(current_object.color, 1.0f);
					box_rectangle.dimensions.size = { 1.0f, 1.0f };
					box_rectangle.filled = true;

					canvas_system.draw_batch_rectangle(m_batch_handle, box_rectangle);
					batch_command.range.count = 1;
				}
				break;
				case RenderObjectType::DIAMOND:
				{
					Vadon::Render::Canvas::Triangle diamond_half_triangle;
					diamond_half_triangle.color = Vadon::Render::Canvas::ColorRGBA(current_object.color, 1.0f);

					diamond_half_triangle.point_a.position = { 0, 1.0f };
					diamond_half_triangle.point_b.position = { 1.0f, 0 };
					diamond_half_triangle.point_c.position = { -1.0f, 0 };

					canvas_system.draw_batch_triangle(m_batch_handle, diamond_half_triangle);

					diamond_half_triangle.point_a.position = { -1.0f, 0 };
					diamond_half_triangle.point_b.position = { 1.0f, 0 };
					diamond_half_triangle.point_c.position = { 0, -1.0f };

					canvas_system.draw_batch_triangle(m_batch_handle, diamond_half_triangle);
					batch_command.range.count = 2;
				}
				break;
				}

				canvas_system.add_item_batch_draw(current_object.canvas_handle, batch_command);
			}
		}

		int32_t register_canvas_item()
		{
			int32_t new_handle = -1;
			if (m_render_free_list.empty() == false)
			{
				new_handle = m_render_free_list.back();
				m_render_free_list.pop_back();
			}
			else
			{
				new_handle = m_active_object_count;
			}
			++m_active_object_count;

			return new_handle;
		}

		void create_canvas_object(int32_t handle, CanvasRenderObject new_object)
		{
			Vadon::Render::Canvas::ItemInfo new_item_info;
			new_item_info.layer = m_canvas_layer;

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			new_object.canvas_handle = canvas_system.create_item(new_item_info);

			if (handle >= m_render_object_pool.size())
			{
				m_render_object_pool.push_back(new_object);
			}
			else
			{
				m_render_object_pool[handle] = new_object;
			}
		}

		void update_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return;
			}

			CanvasRenderObject& render_object = m_render_object_pool[canvas_component->render_handle];
			render_object.color = canvas_component->color;

			const int32_t type = std::clamp(canvas_component->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));

			render_object.type = Vadon::Utilities::to_enum<RenderObjectType>(type);

			reset_canvas_items();
		}

		void remove_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			VadonDemo::Model::CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return;
			}

			// TODO: add async version?
			if (canvas_component->render_handle >= 0)
			{
				internal_remove_canvas_item(m_render_object_pool[canvas_component->render_handle]);
				m_render_free_list.push_back(canvas_component->render_handle);
			}
		}

		void internal_remove_canvas_item(CanvasRenderObject& render_object)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.remove_item(render_object.canvas_handle);

			render_object.canvas_handle.invalidate();

			--m_active_object_count;
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

	void Model::render_sync(Vadon::ECS::World& ecs_world)
	{
		m_internal->render_sync(ecs_world);
	}

	void Model::update_view_async(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue)
	{
		m_internal->update_view_async(ecs_world, render_queue);
	}

	void Model::render_async(const Vadon::Utilities::PacketQueue& render_queue)
	{
		m_internal->render_async(render_queue);
	}

	void Model::lerp_view_state(float lerp_weight)
	{
		m_internal->lerp_view_state(lerp_weight);
	}

	void Model::reset_transforms(Vadon::ECS::World& ecs_world)
	{
		m_internal->reset_transforms(ecs_world);
	}

	void Model::update_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->update_canvas_item(ecs_world, entity);
	}

	void Model::remove_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->remove_canvas_item(ecs_world, entity);
	}

	Vadon::Render::Canvas::LayerHandle Model::get_canvas_layer() const
	{
		return m_internal->m_canvas_layer;
	}

	void Model::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);

		// TODO: register types as needed!
	}
}