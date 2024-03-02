#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>

#include <Vadon/Scene/Node/Node.hpp>

#include <Vadon/Utilities/Data/VariantUtilities.hpp>

#include <format>

namespace VadonEditor::View
{
	namespace
	{
		class PropertyEditor
		{
		public:
			virtual ~PropertyEditor() {}
			virtual bool render(VadonApp::UI::Developer::GUISystem& dev_gui) = 0;

			const std::string& get_name() const { return m_name; }
			const Vadon::Core::Variant& get_value() const { return m_value; }
			void set_value(const Vadon::Core::Variant& value) { m_value = value; value_updated(); }
		protected:
			PropertyEditor(std::string_view property_name, const Vadon::Core::Variant& value)
				: m_name(property_name)
				, m_value(value)
			{}

			virtual void value_updated() = 0;

			std::string m_name;
			Vadon::Core::Variant m_value;
		};

		// FIXME: these all have basically the same API and behavior. Should these be de-duplicated?
		class IntPropertyEditor final : public PropertyEditor
		{
		public:
			IntPropertyEditor(std::string_view property_name, const Vadon::Core::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_input.label = property_name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<int>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				if (dev_gui.draw_input_int(m_input) == true)
				{
					m_value = m_input.input;
					return true;
				}
				return false;
			}
		protected:
			void value_updated() override { m_input.input = std::get<int>(m_value); }
		private:
			UI::Developer::InputInt m_input;
		};

		class FloatPropertyEditor final : public PropertyEditor
		{
		public:
			FloatPropertyEditor(std::string_view property_name, const Vadon::Core::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_input.label = property_name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<float>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				if (dev_gui.draw_input_float(m_input) == true)
				{
					m_value = m_input.input;
					return true;
				}
				return false;
			}
		protected:
			void value_updated() override { m_input.input = std::get<float>(m_value); }
		private:
			UI::Developer::InputFloat m_input;
		};

		class Float2PropertyEditor final : public PropertyEditor
		{
		public:
			Float2PropertyEditor(std::string_view property_name, const Vadon::Core::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_input.label = property_name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<Vadon::Utilities::Vector2>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				if (dev_gui.draw_input_float2(m_input) == true)
				{
					m_value = m_input.input;
					return true;
				}
				return false;
			}
		protected:
			void value_updated() override { m_input.input = std::get<Vadon::Utilities::Vector2>(m_value); }
		private:
			UI::Developer::InputFloat2 m_input;
		};

		class StringPropertyEditor final : public PropertyEditor
		{
		public:
			StringPropertyEditor(std::string_view property_name, const Vadon::Core::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_input.label = property_name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<std::string>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				if (dev_gui.draw_input_text(m_input) == true)
				{
					m_value = m_input.input;
					return true;
				}

				return false;
			}
		protected:
			void value_updated() override { m_input.input = std::get<std::string>(m_value); }
		private:
			UI::Developer::InputText m_input;
		};

		// FIXME: move Node and scene tree editing into dedicated subsystems!
		struct AddNodeDialog
		{
			UI::Developer::Window window;
			
			UI::Developer::Button add_button;
			UI::Developer::Button ok_button;

			UI::Developer::ListBox node_type_list;
			std::vector<std::string> node_class_ids;

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
					// Rebuild node type list
					node_type_list.items.clear();
					node_class_ids.clear();
					
					Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
					const Vadon::Core::ObjectClassInfoList class_info_list = model.get_node_type_list();
					for (const Vadon::Core::ObjectClassInfo& current_class_info : class_info_list)
					{
						node_type_list.items.push_back(current_class_info.name);
						node_class_ids.push_back(current_class_info.id);
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
				return node_class_ids[node_type_list.selected_item];
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

			struct SceneTreeViewState
			{
				Vadon::Scene::Node* clicked_node = nullptr;
			};

			NodeEditor()
			{
				m_window.title = "Node Editor";
				m_window.open = false;

				m_tree_window.id = "Tree";
				m_tree_window.size = Vadon::Utilities::Vector2(400, 300);
				m_tree_window.border = true;

				m_remove_node_button.label = "Remove node";

				m_save_scene_button.label = "Save scene";
				m_load_scene_button.label = "Load scene";
			}

			void render(Core::Editor& editor)
			{
				VadonApp::Core::Application& engine_app = editor.get_engine_app();
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				if (dev_gui.begin_window(m_window) == true)
				{
					Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
					Model::SceneTree& scene_tree = model.get_scene_tree();

					if (scene_tree.get_current_scene().is_valid() == true)
					{
						SceneTreeViewState view_state;
						if (dev_gui.begin_child_window(m_tree_window) == true)
						{
							view_state = render_scene_tree(editor, dev_gui);
						}
						const bool window_clicked = dev_gui.is_window_hovered() && dev_gui.is_mouse_clicked(VadonApp::Platform::MouseButton::LEFT);
						dev_gui.end_child_window();

						if (view_state.clicked_node != nullptr)
						{
							set_selected_node(editor, dev_gui, view_state.clicked_node);
						}
						else if (window_clicked == true)
						{
							set_selected_node(editor, dev_gui, nullptr);
						}

						const bool add_node = m_add_node_dialog.draw(editor, dev_gui);

						if (m_selected_node != nullptr)
						{
							dev_gui.add_text(m_selected_node->get_name());

							if (add_node == true)
							{
								scene_tree.add_node(m_add_node_dialog.get_selected_node_type(), m_selected_node);
							}
							if (dev_gui.draw_button(m_remove_node_button) == true)
							{
								// TODO: prompt user for confirmation!
								scene_tree.delete_node(m_selected_node);
								set_selected_node(editor, dev_gui, nullptr);
							}

							dev_gui.push_id(m_property_editors_id);
							for (auto& current_property : m_property_editors)
							{
								if (current_property->render(dev_gui) == true)
								{
									// Property edited, send update to scene tree and update the editor
									// FIXME: optimize this somehow?
									scene_tree.edit_node_property(*m_selected_node, current_property->get_name(), current_property->get_value());
									current_property->set_value(scene_tree.get_node_property_value(*m_selected_node, current_property->get_name()));
								}
							}
							dev_gui.pop_id();
						}
						else
						{
							if (add_node == true)
							{
								// Add to root node
								scene_tree.add_node(m_add_node_dialog.get_selected_node_type(), m_scene_root);
							}
						}

						Vadon::Scene::Node* scene_root = scene_tree.get_root();
						if (scene_root != nullptr)
						{
							if (dev_gui.draw_button(m_save_scene_button) == true)
							{
								scene_tree.save_scene();
							}
						}
					}
					else
					{
						if (dev_gui.draw_button(m_load_scene_button) == true)
						{
							scene_tree.load_scene();
						}
					}
				}
				dev_gui.end_window();
			}
		private:
			void render_node(Vadon::Scene::Node* node, VadonApp::UI::Developer::GUISystem& dev_gui, SceneTreeViewState& view_state)
			{
				constexpr VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;

				const Vadon::Scene::NodeList& child_nodes = node->get_children();
				VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
				if (m_selected_node == node)
				{
					current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::SELECTED;
				}
				if (child_nodes.empty() == true)
				{
					current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::LEAF;
				}

				const bool node_open = dev_gui.push_tree_node(node, std::format("{} ({})", node->get_name(), node->get_class_id()), current_node_flags);
				if (dev_gui.is_item_clicked() && (dev_gui.is_item_toggled_open() == false))
				{
					view_state.clicked_node = node;
				}

				if (node_open == true)
				{
					for (Vadon::Scene::Node* current_child : child_nodes)
					{
						render_node(current_child, dev_gui, view_state);
					}
					dev_gui.pop_tree_node();
				}
			}

			SceneTreeViewState render_scene_tree(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				SceneTreeViewState tree_view_state;

				Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
				Model::SceneTree& scene_tree = model.get_scene_tree();

				Vadon::Scene::Node* scene_root = scene_tree.get_root();
				if (scene_root == nullptr)
				{
					// No root node
					return tree_view_state;
				}

				render_node(scene_root, dev_gui, tree_view_state);

				return tree_view_state;
			}

			void set_selected_node(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui, Vadon::Scene::Node* node)
			{
				if (m_selected_node == node)
				{
					return;
				}

				m_selected_node = node;
				if (m_selected_node == nullptr)
				{
					return;
				}

				Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
				Model::SceneTree& scene_tree = model.get_scene_tree();

				m_property_editors.clear();

				m_property_editors_id = dev_gui.get_id(m_selected_node->get_node_path());

				const Vadon::Core::ObjectPropertyList selected_node_properties = scene_tree.get_node_properties(*m_selected_node);
				for (const Vadon::Core::ObjectProperty& current_property : selected_node_properties)
				{
					auto property_visitor = Vadon::Utilities::VisitorOverloadList{
						[this, &current_property](int)
						{
							m_property_editors.emplace_back(new IntPropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](float)
						{
							m_property_editors.emplace_back(new FloatPropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](Vadon::Utilities::Vector2)
						{
							m_property_editors.emplace_back(new Float2PropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](std::string)
						{
							m_property_editors.emplace_back(new StringPropertyEditor(current_property.name, current_property.value));
						},
						[](auto){ /* TODO: error? */ }
					};

					std::visit(property_visitor, current_property.value);
				}
			}

			UI::Developer::Window m_window;

			Vadon::Scene::Node* m_scene_root = nullptr;

			Vadon::Scene::Node* m_selected_node = nullptr;
			VadonApp::UI::Developer::GUISystem::ID m_property_editors_id = 0;
			std::vector<std::unique_ptr<PropertyEditor>> m_property_editors;

			UI::Developer::ChildWindow m_tree_window;

			AddNodeDialog m_add_node_dialog;
			UI::Developer::Button m_remove_node_button;
			UI::Developer::Button m_save_scene_button;
			UI::Developer::Button m_load_scene_button; // FIXME: add proper file logic!
			UI::Developer::ListBox m_node_type_list; // FIXME: replace with tree view, branching on subclasses!
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