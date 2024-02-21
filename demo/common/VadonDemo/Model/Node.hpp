#ifndef VADONDEMO_MODEL_NODE_HPP
#define VADONDEMO_MODEL_NODE_HPP
#include <Vadon/Scene/Node/Node.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace VadonDemo::Model
{
	using Node = Vadon::Scene::Node;

	struct DrawData
	{
		Vadon::Utilities::Vector2 position;
		float scale;
	};

	class TestNode : public Node
	{
		VADON_OBJECT_CLASS(TestNode, Node)
	public:
		TestNode(Vadon::Core::EngineCoreInterface& core) : Node(core) {}

		void initialize() override;
		void update(float delta_time) override;

		const Vadon::Utilities::Vector2& get_position() const { return m_position; }
		const float get_scale() const { return m_scale; }
	private:
		Vadon::Utilities::Vector2 m_position = { 0, 0 };
		Vadon::Utilities::Vector2 m_origin = { 0, 0 };
		float m_rotation = 0.0f;
		float m_angular_velocity = 1.0f;
		float m_radius = 0.0f;
		float m_scale = 1.0f;
	};

	class TestSystem : public Node
	{
		VADON_OBJECT_CLASS(TestSystem, Node)
	public:
		TestSystem(Vadon::Core::EngineCoreInterface& core) : Node(core) {}
		void initialize() override;

		std::vector<DrawData> render() const;
	private:
		std::vector<TestNode*> m_test_nodes;
	};
}
#endif