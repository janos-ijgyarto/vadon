#include <VadonDemo/Model/Node.hpp>

#include <random>

namespace VadonDemo::Model
{
	namespace
	{
		float get_random_float(float min, float max)
		{
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
		}
	}

	void TestNode::initialize()
	{
		m_origin.x = get_random_float(-300.0f, 300.0f);
		m_origin.y = get_random_float(-300.0f, 300.0f);

		m_radius = get_random_float(10.0f, 100.0f);
		m_angular_velocity = get_random_float(float(std::_Pi_val) / 6.0f, float(std::_Pi_val) / 2.0f);

		m_position = m_origin;
		m_position.y += m_radius;

		m_scale = get_random_float(0.25f, 2.5f);
	}

	void TestNode::update(float delta_time)
	{
		m_rotation += delta_time * m_angular_velocity;
		if (m_rotation > (float(std::_Pi_val) * 2.0f))
		{
			m_rotation = std::fmodf(m_rotation, float(std::_Pi_val) * 2.0f);
		}

		m_position.x = m_origin.x + (std::cosf(m_rotation) * m_radius);
		m_position.y = m_origin.y + (std::sinf(m_rotation) * m_radius);
	}

	void TestSystem::initialize()
	{
		for (Node* current_child : get_children())
		{
			// FIXME: replace with custom RTTI!
			TestNode* test_child = dynamic_cast<TestNode*>(current_child);
			if (test_child != nullptr)
			{
				m_test_nodes.push_back(test_child);
			}
		}
	}

	std::vector<DrawData> TestSystem::render() const
	{
		std::vector<DrawData> draw_data;
		for (const TestNode* current_node : m_test_nodes)
		{
			DrawData& current_draw_data = draw_data.emplace_back();
			current_draw_data.position = current_node->get_position();
			current_draw_data.scale = current_node->get_scale();
		}

		return draw_data;
	}
}