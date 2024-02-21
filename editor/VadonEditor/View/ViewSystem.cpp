#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>

#include <format>

namespace VadonEditor::View
{
	namespace
	{
		struct AddNodeDialog
		{
			UI::Developer::Window window;
			
			UI::Developer::Button add_button;
			UI::Developer::Button ok_button;

			UI::Developer::ListBox node_type_list;

			AddNodeDialog()
			{
				window.title = "Add Node";

				add_button.label = "Add node";
				ok_button.label = "OK";

				node_type_list.label = "Node types";
			}

			bool draw(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.draw_button(add_button) == true)
				{
					node_type_list.items.clear();
					
					Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
					for (const Vadon::Core::ObjectClassInfo& current_class_info : model.get_node_class_list())
					{
						node_type_list.items.push_back(current_class_info.name);
					}

					dev_gui.open_dialog(window.title);
				}

				bool node_added = false;
				if (dev_gui.begin_modal_dialog(window) == true)
				{
					dev_gui.draw_list_box(node_type_list);
					if (dev_gui.draw_button(ok_button) == true)
					{
						node_added = true;
						dev_gui.close_current_dialog();
					}

					dev_gui.end_dialog();
				}

				return node_added;
			}

			std::string get_selected_node_type() const
			{
				return node_type_list.items[node_type_list.selected_item];
			}
		};

		class NodeEditor
		{
		public:
			struct EditorNode
			{
				std::string name;
				std::string type_id;
				EditorNode* parent = nullptr;
				std::vector<EditorNode*> children;

				void add_child(std::string_view type_id_str)
				{
					EditorNode* new_child = new EditorNode();
					new_child->name = "Node";
					new_child->type_id = type_id_str;
					new_child->parent = this;

					children.push_back(new_child);
				}

				void remove_child(EditorNode* child)
				{
					auto child_it = std::find(children.begin(), children.end(), child);
					if (child_it != children.end())
					{
						children.erase(child_it);
						delete child;
					}
				}
			};

			NodeEditor()
			{
				m_window.title = "Node Editor";
				m_window.open = false;

				m_root_node.name = "Root";
				m_root_node.type_id = "Node";

				m_tree_window.id = "Tree";
				m_tree_window.size = Vadon::Utilities::Vector2(400, 300);
				m_tree_window.border = true;

				m_remove_node_button.label = "Remove node";
				m_node_name_input.label = "Node name";
			}

			void render(Core::Editor& editor)
			{
				VadonApp::Core::Application& engine_app = editor.get_engine_app();
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				if (dev_gui.begin_window(m_window) == true)
				{
					EditorNode* clicked_node = nullptr;
					if (dev_gui.begin_child_window(m_tree_window) == true)
					{
						const VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;
						if (dev_gui.push_tree_node(&m_root_node, std::format("{} ({})", m_root_node.name, m_root_node.type_id), node_base_flags) == true)
						{
							struct NodeStackEntry
							{
								EditorNode* node = nullptr;
								size_t child_index = 0;
							};

							using NodeStack = std::vector<NodeStackEntry>;

							static NodeStack node_stack;
							{
								NodeStackEntry& root_entry = node_stack.emplace_back();
								root_entry.node = &m_root_node;
							}

							while (node_stack.empty() == false)
							{
								NodeStackEntry& current_entry = node_stack.back();
								if (current_entry.child_index < current_entry.node->children.size())
								{
									NodeStackEntry child_entry;
									child_entry.node = current_entry.node->children[current_entry.child_index];

									VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
									if (selected_node == child_entry.node)
									{
										current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::SELECTED;
									}
									if (child_entry.node->children.empty() == true)
									{
										current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::LEAF;
									}

									const bool node_open = dev_gui.push_tree_node(child_entry.node, std::format("{} ({})", child_entry.node->name, child_entry.node->type_id), current_node_flags);
									if (dev_gui.is_item_clicked() && (dev_gui.is_item_toggled_open() == false))
									{
										clicked_node = child_entry.node;
									}
									if (node_open == true)
									{
										node_stack.push_back(child_entry);
									}
									++current_entry.child_index;
								}
								else
								{
									dev_gui.pop_tree_node();
									node_stack.pop_back();
								}
							}

							node_stack.clear();
						}
					}
					const bool window_clicked = dev_gui.is_window_hovered() && dev_gui.is_mouse_clicked(VadonApp::Platform::MouseButton::LEFT);
					dev_gui.end_child_window();

					if (clicked_node != nullptr)
					{
						selected_node = clicked_node;
						update_selected_node_label();
					}
					else if (window_clicked == true)
					{
						selected_node = nullptr;
					}

					const bool add_node = m_add_node_dialog.draw(editor, dev_gui);

					if (selected_node != nullptr)
					{
						dev_gui.add_text(selected_node_label);

						if (add_node == true)
						{
							selected_node->add_child(m_add_node_dialog.get_selected_node_type());
						}
						if (dev_gui.draw_button(m_remove_node_button) == true)
						{
							selected_node->parent->remove_child(selected_node);
							selected_node = nullptr;
						}
						if (dev_gui.draw_input_text(m_node_name_input) == true)
						{
							selected_node->name = m_node_name_input.input;
							update_selected_node_label();
						}
					}
					else
					{
						if (add_node == true)
						{
							m_root_node.add_child(m_add_node_dialog.get_selected_node_type());
						}
					}
				}
				dev_gui.end_window();
			}

			void update_selected_node_label()
			{
				m_node_name_input.input = selected_node->name;

				std::string node_path = selected_node->name;
				const EditorNode* parent = selected_node->parent;
				while (parent != nullptr)
				{
					node_path = parent->name + "/" + node_path;
					parent = parent->parent;
				}

				selected_node_label = "Node path: " + node_path;
			}
		private:
			UI::Developer::Window m_window;
			EditorNode m_root_node;

			EditorNode* selected_node = nullptr;
			std::string selected_node_label;

			UI::Developer::ChildWindow m_tree_window;

			AddNodeDialog m_add_node_dialog;
			UI::Developer::Button m_remove_node_button;
			UI::Developer::InputText m_node_name_input;
			UI::Developer::ListBox m_node_type_list;
		};
	}

	struct ViewSystem::Internal
	{
		Core::Editor& m_editor;

		NodeEditor m_node_editor;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{
		}

		bool initialize()
		{
			m_editor.get_system<UI::UISystem>().register_ui_element([this](Core::Editor& editor) { m_node_editor.render(editor); });
			return true;
		}

		void update()
		{
			// TODO
		}
	};

	ViewSystem::~ViewSystem() = default;

	ViewSystem::ViewSystem(Core::Editor& editor)
		: m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool ViewSystem::initialize()
	{
		return m_internal->initialize();
	}

	void ViewSystem::update()
	{
		m_internal->update();
	}
}