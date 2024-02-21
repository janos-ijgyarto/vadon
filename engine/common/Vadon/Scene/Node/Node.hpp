#ifndef VADON_SCENE_NODE_NODE_HPP
#define VADON_SCENE_NODE_NODE_HPP
#include <Vadon/Core/Object/Object.hpp>
#include <vector>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace Vadon::Scene
{
	class Node;
	using NodeList = std::vector<Node*>;

	class SceneSystem;

	// Inspired by Godot Nodes
	class Node : public Core::Object
	{
	public:
		VADON_OBJECT_CLASS(Vadon::Scene::Node, Vadon::Core::Object)

		VADONCOMMON_API Node(Core::EngineCoreInterface& core);
		VADONCOMMON_API virtual ~Node();

		virtual void initialize() {}
		virtual void update(float) {}

		VADONCOMMON_API void add_child(Node* node, int32_t index = -1);
		VADONCOMMON_API void remove_child(Node* node);

		Node* get_parent() const { return m_parent; }
		const NodeList& get_children() const { return m_children; }

		std::string get_name() const { return m_name; }
		void set_name(std::string_view name) { m_name = name; }
	protected:
		VADONCOMMON_API static void bind_methods(Core::ObjectClassData& class_data);
	private:
		void internal_remove_child(Node* node, bool reparent);
		void set_parent(Node* parent);

		void enter_tree();
		void exit_tree();

		Node* m_parent = nullptr;
		bool m_in_tree = false; // FIXME: use a flag, or use recursive query?

		NodeList m_children;
		std::string m_name;

		friend SceneSystem;
	};
}
#endif