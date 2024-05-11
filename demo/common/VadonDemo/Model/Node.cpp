#include <VadonDemo/Model/Node.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/Core/Object/ClassData.hpp>
#include <Vadon/Core/Object/ObjectSystem.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <numbers>

namespace VadonDemo::Model
{
	namespace
	{
		constexpr float c_pi = std::numbers::pi_v<float>;

		Vadon::Utilities::Vector2 rotate_vector(const Vadon::Utilities::Vector2& vector, float angle)
		{
			const float sin_angle = std::sinf(angle);
			const float cos_angle = std::cosf(angle);

			return Vadon::Utilities::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
		}
	}

	void CanvasLayer::initialize()
	{
		Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		m_layer = canvas_system.create_layer(m_info);
	}

	void CanvasLayer::set_offset(const Vadon::Utilities::Vector2& offset)
	{
		m_info.offset = offset;
		if (m_layer.is_valid() == true)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.set_layer_offset(m_layer, offset);
		}
	}

	void CanvasLayer::bind_methods(Vadon::Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_offset)>("get_offset");
		class_data.bind_method<VADON_METHOD_BIND(set_offset)>("set_offset");

		class_data.add_property(VADON_ADD_OBJECT_PROPERTY("offset", Vadon::Utilities::Vector2, "get_offset", "set_offset"));
	}

	void Node2D::initialize()
	{
		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		m_parent_2d = object_system.get_object_as<Node2D>(*get_parent());

		Node* parent = get_parent();
		while (parent != nullptr)
		{
			CanvasLayer* canvas_layer_parent = object_system.get_object_as<CanvasLayer>(*parent);
			if (canvas_layer_parent != nullptr)
			{
				Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
				
				Vadon::Render::Canvas::ItemInfo canvas_item_info;
				canvas_item_info.layer = canvas_layer_parent->get_layer_handle();
				canvas_item_info.position = m_global_position;
				
				m_canvas_item = canvas_system.create_item(canvas_item_info);
				break;
			}

			parent = parent->get_parent();
		}

		update_global_position();
	}

	void Node2D::update(float /*delta_time*/)
	{
		update_global_position();
	}

	void Node2D::bind_methods(Vadon::Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_position)>("get_position");
		class_data.bind_method<VADON_METHOD_BIND(set_position)>("set_position");

		class_data.bind_method<VADON_METHOD_BIND(get_scale)>("get_scale");
		class_data.bind_method<VADON_METHOD_BIND(set_scale)>("set_scale");

		class_data.add_property(VADON_ADD_OBJECT_MEMBER_PROPERTY("position", m_position, "get_position", "set_position"));
		class_data.add_property(VADON_ADD_OBJECT_MEMBER_PROPERTY("scale", m_scale, "get_scale", "set_scale"));
	}

	void Node2D::update_global_position()
	{
		// FIXME: this is just a really hacky way to get this working
		if (m_parent_2d != nullptr)
		{
			m_global_position = m_parent_2d->get_global_position() + m_position;

			if (m_canvas_item.is_valid() == true)
			{
				Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
				canvas_system.set_item_position(m_canvas_item, m_global_position);
			}
		}
		else
		{
			m_global_position = m_position;
		}
	}

	void Orbiter::initialize()
	{
		Node2D::initialize();

		m_color = Vadon::Utilities::Vector4(0, 1, 0, 1);

		if (m_parent_2d != nullptr)
		{
			Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
			Pivot* pivot_parent = object_system.get_object_as<Pivot>(*m_parent_2d);
			if (pivot_parent != nullptr)
			{
				m_pivot_parent = true;
			}
		}

		if (m_canvas_item.is_valid() == true)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			Vadon::Render::Canvas::Rectangle orbiter_rectangle;
			orbiter_rectangle.color = m_color;
			orbiter_rectangle.dimensions.size = { m_scale, m_scale };
			orbiter_rectangle.filled = true;

			canvas_system.draw_rectangle(m_canvas_item, orbiter_rectangle);

			canvas_system.set_item_position(m_canvas_item, m_global_position);
		}
	}

	void Orbiter::update(float delta_time)
	{
		if (m_pivot_parent == true)
		{
			// Rotate around pivot
			m_position = rotate_vector(m_position, delta_time * m_angular_velocity);
		}

		Node2D::update(delta_time);
	}

	void Orbiter::bind_methods(Vadon::Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_angular_velocity)>("get_angular_velocity");
		class_data.bind_method<VADON_METHOD_BIND(set_angular_velocity)>("set_angular_velocity");

		class_data.add_property(VADON_ADD_OBJECT_MEMBER_PROPERTY("angular_velocity", m_angular_velocity, "get_angular_velocity", "set_angular_velocity"));
	}

	void Pivot::initialize()
	{
		Node2D::initialize();
		m_color = Vadon::Utilities::Vector4(1, 0, 0, 1);
		if (m_parent_2d != nullptr)
		{
			Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
			Pivot* pivot_parent = object_system.get_object_as<Pivot>(*m_parent_2d);
			if (pivot_parent != nullptr)
			{
				m_pivot_parent = true;
			}
		}

		if (m_canvas_item.is_valid() == true)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			
			Vadon::Render::Canvas::Triangle pivot_triangle;
			pivot_triangle.color = m_color;

			pivot_triangle.point_a.position = { 0, m_scale };
			pivot_triangle.point_b.position = { m_scale, 0 };
			pivot_triangle.point_c.position = { -m_scale, 0 };

			canvas_system.draw_triangle(m_canvas_item, pivot_triangle);

			pivot_triangle.point_a.position = { -m_scale, 0 };
			pivot_triangle.point_b.position = { m_scale, 0 };
			pivot_triangle.point_c.position = { 0, -m_scale };

			canvas_system.draw_triangle(m_canvas_item, pivot_triangle);

			canvas_system.set_item_position(m_canvas_item, m_global_position);
		}
	}

	void Pivot::update(float delta_time)
	{
		if (m_pivot_parent == true)
		{
			// Rotate around pivot
			constexpr float c_pivot_velocity = 1.0f;
			m_position = rotate_vector(m_position, delta_time * c_pivot_velocity);
		}

		Node2D::update(delta_time);
	}
}