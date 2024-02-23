#include <VadonDemo/Model/Node.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/Core/Object/ClassData.hpp>
#include <Vadon/Core/Object/ObjectSystem.hpp>

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

	void Node2D::initialize()
	{
		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		m_parent_2d = object_system.get_object_as<Node2D>(*get_parent());
	}

	void Node2D::update(float /*delta_time*/)
	{
		// FIXME: this is just a really hacky way to get this working
		if (m_parent_2d != nullptr)
		{
			m_global_position = m_parent_2d->get_global_position() + m_position;
		}
		else
		{
			m_global_position = m_position;
		}
	}

	void Node2D::bind_methods(Vadon::Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_position)>("get_position");
		class_data.bind_method<VADON_METHOD_BIND(set_position)>("set_position");

		class_data.bind_method<VADON_METHOD_BIND(get_scale)>("get_scale");
		class_data.bind_method<VADON_METHOD_BIND(set_scale)>("set_scale");

		class_data.add_property(VADON_ADD_OBJECT_PROPERTY("position", m_position, "get_position", "set_position"));
		class_data.add_property(VADON_ADD_OBJECT_PROPERTY("scale", m_scale, "get_scale", "set_scale"));
	}

	void Orbiter::initialize()
	{
		Node2D::initialize();

		m_color = Vadon::Utilities::Vector4(0, 1, 0, 1);

		if (m_parent_2d == nullptr)
		{
			return;
		}

		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		Pivot* pivot_parent = object_system.get_object_as<Pivot>(*m_parent_2d);
		if (pivot_parent != nullptr)
		{
			m_pivot_parent = true;
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

		class_data.add_property(VADON_ADD_OBJECT_PROPERTY("angular_velocity", m_angular_velocity, "get_angular_velocity", "set_angular_velocity"));
	}

	void Pivot::initialize()
	{
		Node2D::initialize();
		m_color = Vadon::Utilities::Vector4(1, 0, 0, 1);
		if (m_parent_2d == nullptr)
		{
			return;
		}

		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		Pivot* pivot_parent = object_system.get_object_as<Pivot>(*m_parent_2d);
		if (pivot_parent != nullptr)
		{
			m_pivot_parent = true;
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