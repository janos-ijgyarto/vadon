#include <VadonEditor/View/Scene/SceneTree.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Model/Scene/SceneTree.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/View/Scene/ECS/Entity.hpp>
#include <VadonEditor/View/Scene/Resource/Scene.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <format>

namespace VadonEditor::View
{
	class InstantiateSceneDialog : public LoadSceneDialog
	{
	public:
		InstantiateSceneDialog(Core::Editor& editor)
			: LoadSceneDialog(editor, "Instantiate Sub-Scene")
		{
			m_load_button.label = "Instantiate";
		}
	};

	struct SceneTreeWindow::Internal
	{
		UI::Developer::Window m_window;

		VadonApp::UI::Developer::MessageDialog m_error_dialog;

		NewSceneDialog m_new_scene_dialog;
		LoadSceneDialog m_load_scene_dialog;
		InstantiateSceneDialog m_instantiate_scene_dialog;
		
		UI::Developer::Button m_add_entity_button;
		UI::Developer::Button m_instantiate_scene_button;
		UI::Developer::Button m_remove_entity_button;

		UI::Developer::ChildWindow m_tree_window;
		std::string m_tree_label_cache;

		EntityEditor m_entity_editor;

		struct TreeViewState
		{
			VadonEditor::Model::Entity* clicked_entity = nullptr;
		};

		Internal(Core::Editor& editor)
			: m_entity_editor(editor)
			, m_load_scene_dialog(editor)
			, m_instantiate_scene_dialog(editor)
		{
			m_window.title = "Scene Tree";
			m_window.open = false;

			m_add_entity_button.label = "Add entity";
			m_instantiate_scene_button.label = "Instantiate Child Scene";
			m_remove_entity_button.label = "Remove entity";

			m_tree_window.id = "Tree";
			m_tree_window.size = Vadon::Utilities::Vector2(400, 300);
			m_tree_window.border = true;
		}

		void draw(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			if (dev_gui.begin_window(m_window) == true)
			{				
				Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
				Model::SceneTree& scene_tree = model.get_scene_tree();

				m_error_dialog.draw(dev_gui);

				if (m_new_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
				{
					if (scene_tree.new_scene(m_new_scene_dialog.get_new_scene_name()) == false)
					{
						show_error("Scene Error", "Error creating scene!");
					}
				}
				if (m_load_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
				{
					if (scene_tree.load_scene(m_load_scene_dialog.get_loaded_scene()) == false)
					{
						show_error("Scene Error", "Error loading scene!");
					}
					else
					{
						// Reset editor to make sure we're not getting properties from an invalid entity
						m_entity_editor.reset();
					}
				}

				if (scene_tree.get_current_scene().is_valid() == true)
				{
					// TODO: dialog for creating entity?
					const bool add_entity = dev_gui.draw_button(m_add_entity_button);
					if (m_entity_editor.m_entity != nullptr)
					{
						if (dev_gui.draw_button(m_remove_entity_button) == true)
						{
							// TODO: prompt user for confirmation!
							scene_tree.remove_entity(m_entity_editor.m_entity);
							m_entity_editor.set_selected_entity(nullptr);
						}
						else if (add_entity == true)
						{
							scene_tree.add_entity(m_entity_editor.m_entity);
						}
					}
					else if (add_entity == true)
					{
						scene_tree.add_entity();
					}

					if (m_entity_editor.m_entity != nullptr)
					{
						if(dev_gui.draw_button(m_instantiate_scene_button) == true)
						{
							m_instantiate_scene_dialog.open();
						}

						if (m_instantiate_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
						{
							if (scene_tree.instantiate_sub_scene(m_entity_editor.m_entity, m_instantiate_scene_dialog.get_loaded_scene()) == false)
							{
								show_error("Scene Error", "Error instantiating sub-scene!");
							}
						}
					}

					TreeViewState view_state;
					if (dev_gui.begin_child_window(m_tree_window) == true)
					{
						view_state = render_scene_tree(editor, dev_gui);
					}
					const bool window_clicked = dev_gui.is_window_hovered() && dev_gui.is_mouse_clicked(VadonApp::Platform::MouseButton::LEFT);
					dev_gui.end_child_window();

					if (view_state.clicked_entity != nullptr)
					{
						m_entity_editor.set_selected_entity(view_state.clicked_entity);
					}
					else if (window_clicked == true)
					{
						m_entity_editor.set_selected_entity(nullptr);
					}
				}
			}
			dev_gui.end_window();

			m_entity_editor.draw(dev_gui);
		}

		void reset()
		{
			m_entity_editor.reset();
		}

		void render_entity_node(Model::Entity* entity, VadonApp::UI::Developer::GUISystem& dev_gui, TreeViewState& view_state)
		{
			constexpr VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;

			VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
			if (m_entity_editor.m_entity == entity)
			{
				current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::SELECTED;
			}
			if (entity->has_visible_children() == false)
			{
				current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::LEAF;
			}

			std::string_view entity_name = entity->get_name();
			if (entity->is_sub_scene() == true)
			{
				m_tree_label_cache.clear();
				std::format_to(std::back_inserter(m_tree_label_cache), "{} ({})", entity->get_name(), entity->get_sub_scene_info().resource_path.path);
				entity_name = m_tree_label_cache;
			}

			const bool node_open = dev_gui.push_tree_node(entity, entity_name, current_node_flags);
			if (dev_gui.is_item_clicked() && (dev_gui.is_item_toggled_open() == false))
			{
				view_state.clicked_entity = entity;
			}

			if (node_open == true)
			{
				const Model::EntityList& child_entities = entity->get_children();
				for (Model::Entity* current_child : child_entities)
				{
					if (current_child->is_sub_scene_child() == false)
					{
						render_entity_node(current_child, dev_gui, view_state);
					}
				}
				dev_gui.pop_tree_node();
			}
		}

		TreeViewState render_scene_tree(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			TreeViewState tree_view_state;

			Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
			Model::SceneTree& scene_tree = model.get_scene_tree();

			Model::Entity* scene_root = scene_tree.get_root();
			if (scene_root == nullptr)
			{
				// No root entity
				return tree_view_state;
			}

			render_entity_node(scene_root, dev_gui, tree_view_state);

			return tree_view_state;
		}

		void show_error(std::string_view title, std::string_view message)
		{
			m_error_dialog.set_title(title);
			m_error_dialog.set_message(message);
			m_error_dialog.open();
		}

		void on_new_scene_action()
		{
			// TODO: only open if no other dialog is open?
			// TODO2: prompt on unsaved changes in scene?
			m_new_scene_dialog.open();
		}

		void on_load_scene_action()
		{
			// TODO: only open if no other dialog is open?
			// TODO2: prompt on unsaved changes in scene?
			m_load_scene_dialog.open();
		}

		void on_save_scene_action(Core::Editor& editor)
		{
			Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
			Model::SceneTree& scene_tree = model.get_scene_tree();
						
			Model::Entity* scene_root = scene_tree.get_root();
			if (scene_root != nullptr)
			{
				scene_tree.save_scene();
			}
			else
			{
				show_error("Scene Error", "Cannot save empty scene!");
			}
		}
	};

	SceneTreeWindow::~SceneTreeWindow() = default;

	SceneTreeWindow::SceneTreeWindow(Core::Editor& editor)
		: m_editor(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{

	}

	void SceneTreeWindow::on_new_scene_action()
	{
		m_internal->on_new_scene_action();
	}

	void SceneTreeWindow::on_load_scene_action()
	{
		m_internal->on_load_scene_action();
	}

	void SceneTreeWindow::on_save_scene_action()
	{
		m_internal->on_save_scene_action(m_editor);
	}

	void SceneTreeWindow::draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		m_internal->draw(m_editor, dev_gui);
	}
}