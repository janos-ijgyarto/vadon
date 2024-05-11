#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Scene/Node/Node.hpp>

#include <Vadon/Core/Object/ClassData.hpp>

#include <unordered_map>
#include <format>

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
		if (node->is_ancestor_of(*this) == true)
		{
			// TODO: throw error?
			return;
		}

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

		enforce_unique_child_name(node);

		node->set_parent(this);
	}

	void Node::remove_child(Node* node)
	{
		internal_remove_child(node, false);
	}

	void Node::set_name(std::string_view name)
	{
		m_name = name;
		if (m_parent != nullptr)
		{
			m_parent->enforce_unique_child_name(this);
		}
	}

	bool Node::is_ancestor_of(const Node& node) const
	{
		const Node* parent = node.get_parent();
		while (parent != nullptr)
		{
			if (parent == this)
			{
				return true;
			}

			parent = parent->get_parent();
		}

		return false;
	}

	std::string Node::get_node_path() const
	{
		std::string path;

		Node* parent = get_parent();
		while (parent)
		{
			path = parent->get_name() + "/" + path;
			parent = parent->get_parent();
		}

		if (path.empty() == false)
		{
			// Trim the trailing slash
			path.pop_back();
		}
		else
		{
			path = ".";
		}

		return path;
	}

	void Node::bind_methods(Core::ObjectClassData& class_data)
	{
		class_data.bind_method<VADON_METHOD_BIND(get_name)>("get_name");
		class_data.bind_method<VADON_METHOD_BIND(set_name)>("set_name");

		class_data.add_property(VADON_ADD_OBJECT_MEMBER_PROPERTY("name", m_name, "get_name", "set_name"));
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

	void Node::enforce_unique_child_name(Node* child)
	{
		auto child_name_search = [this, &child](std::string_view unique_name)
			{
				for (Node* current_child : m_children)
				{
					if ((current_child != child) && (current_child->m_name == unique_name))
					{
						return true;
					}
				}

				return false;
			};

		size_t attempt_count = 1;
		std::string unique_child_name = child->m_name;
		while (child_name_search(unique_child_name) == true)
		{
			++attempt_count;
			unique_child_name = std::format("{}_{}", child->get_name(), attempt_count);
		}

		child->m_name = unique_child_name;
	}
}