#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Scene/Node/Node.hpp>

#include <Vadon/Core/Object/ClassData.hpp>

namespace Vadon::Scene
{
	Node::Node(Core::EngineCoreInterface& core)
		: Core::Object(core)
	{

	}

	Node::~Node()
	{
		for (Node* current_child : m_children)
		{
			delete current_child;
		}
	}

	void Node::add_child(Node* node, int32_t index)
	{
		// TODO: recursion check
		if (node->m_parent != nullptr)
		{
			node->m_parent->internal_remove_child(node, true);
		}

		if ((index >= 0) && (index < static_cast<int32_t>(m_children.size())))
		{
			m_children.insert(m_children.begin() + index, node);
		}
		else
		{
			// Add to end
			m_children.push_back(node);
		}

		node->set_parent(this);
	}

	void Node::remove_child(Node* node)
	{
		internal_remove_child(node, false);
	}

	void Node::bind_methods(Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_name)>("get_name");
		class_data.bind_method<VADON_METHOD_BIND(set_name)>("set_name");
	}

	void Node::internal_remove_child(Node* node, bool reparent)
	{
		auto child_it = std::find(m_children.begin(), m_children.end(), node);
		if (child_it != m_children.end())
		{
			m_children.erase(child_it);
			if (reparent == false)
			{
				node->set_parent(nullptr);
			}
		}
	}

	void Node::set_parent(Node* parent)
	{
		if (parent != nullptr)
		{
			m_parent = parent;
			if ((m_in_tree == false) && (parent->m_in_tree == true))
			{
				enter_tree();
			}
		}
		else
		{
			exit_tree();
		}
	}

	void Node::enter_tree()
	{
		m_in_tree = true;
		// FIXME: enqueue in node system, don't run here!
		initialize();
		for (Node* current_child : m_children)
		{
			// FIXME: do this without recursion?
			current_child->enter_tree();
		}
	}

	void Node::exit_tree()
	{
		if (m_in_tree == true)
		{
			m_in_tree = false;
			for (Node* current_child : m_children)
			{
				// FIXME: do this without recursion?
				current_child->exit_tree();
			}
		}
	}
}