#ifndef VADONDEMO_MODEL_NODE_HPP
#define VADONDEMO_MODEL_NODE_HPP
#include <VadonDemo/VadonDemoCommon.hpp>

#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>

#include <Vadon/Scene/Node/Node.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace VadonDemo::Model
{
	using Node = Vadon::Scene::Node;

	class CanvasLayer : public Node
	{
		VADON_OBJECT_CLASS(CanvasLayer, Node)
	public:
		CanvasLayer(Vadon::Core::EngineCoreInterface& core) : Node(core) {}
		VADONDEMO_API void initialize() override;

		Vadon::Utilities::Vector2 get_offset() const { return m_info.offset; }
		void set_offset(const Vadon::Utilities::Vector2& offset);

		Vadon::Render::Canvas::LayerHandle get_layer_handle() const { return m_layer; }

		// TODO: layer flags!
	protected:
		VADONDEMO_API static void bind_methods(Vadon::Core::ObjectClassData& class_data);
	private:
		Vadon::Render::Canvas::LayerInfo m_info;
		Vadon::Render::Canvas::LayerHandle m_layer;
	};

	class Node2D : public Node
	{
		VADON_OBJECT_CLASS(Node2D, Node)
	public:
		Node2D(Vadon::Core::EngineCoreInterface& core) : Node(core) {}

		VADONDEMO_API void initialize() override;
		VADONDEMO_API void update(float delta_time) override;

		Vadon::Utilities::Vector2 get_position() const { return m_position; }
		void set_position(const Vadon::Utilities::Vector2& position) { m_position = position; }

		Vadon::Utilities::Vector2 get_global_position() const { return m_global_position; }

		float get_scale() const { return m_scale; }
		void set_scale(float scale) { m_scale = scale; }

		Vadon::Utilities::Vector4 get_color() const { return m_color; }
	protected:
		std::vector<Vadon::Utilities::Vector2> m_child_offsets; // FIXME: implement proper 2D transform hierarchy, for now we do it "by hand"

		VADONDEMO_API static void bind_methods(Vadon::Core::ObjectClassData& class_data);
		void update_global_position();

		Vadon::Utilities::Vector2 m_position = Vadon::Utilities::Vector2_Zero;
		Vadon::Utilities::Vector2 m_global_position = Vadon::Utilities::Vector2_Zero;
		float m_scale = 1.0f;
		Vadon::Utilities::Vector4 m_color = Vadon::Utilities::Vector4_One;

		Node2D* m_parent_2d = nullptr;
		Vadon::Render::Canvas::ItemHandle m_canvas_item;
	};

	class Orbiter : public Node2D
	{
		VADON_OBJECT_CLASS(Orbiter, Node2D)
	public:
		Orbiter(Vadon::Core::EngineCoreInterface& core) : Node2D(core) {}

		VADONDEMO_API void initialize() override;
		VADONDEMO_API void update(float delta_time) override;

		float get_angular_velocity() const { return m_angular_velocity; }
		void set_angular_velocity(float angular_velocity) { m_angular_velocity = angular_velocity; }
	protected:
		VADONDEMO_API static void bind_methods(Vadon::Core::ObjectClassData& class_data);
	private:
		float m_angular_velocity = 1.0f;

		bool m_pivot_parent = false;
	};

	class Pivot : public Node2D
	{
		VADON_OBJECT_CLASS(Pivot, Node2D)
	public:
		Pivot(Vadon::Core::EngineCoreInterface& core) : Node2D(core) {}
		VADONDEMO_API void initialize() override;
		VADONDEMO_API void update(float delta_time) override;
	private:
		bool m_pivot_parent = false;
	};
}
#endif