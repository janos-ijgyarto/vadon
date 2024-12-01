#include <VadonEditor/View/Scene/SceneTree.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/View/ViewSystem.hpp>
#include <VadonEditor/View/Scene/Resource/Scene.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <format>

namespace VadonEditor::View
{
	class InstantiateSceneDialog : public SelectSceneDialog
	{
	public:
		InstantiateSceneDialog(Core::Editor& editor)
			: SelectSceneDialog(editor, "Instantiate Sub-Scene")
		{
			m_select_button.label = "Instantiate";
		}
	};

	struct SceneTreeWindow::Internal
	{
		UI::Developer::Window m_window;

		VadonApp::UI::Developer::MessageDialog m_error_dialog;

		InstantiateSceneDialog m_instantiate_scene_dialog;
		
		UI::Developer::Button m_add_entity_button;
		UI::Developer::Button m_instantiate_scene_button;
		UI::Developer::Button m_remove_entity_button;

		UI::Developer::ChildWindow m_tree_window;
		std::string m_tree_label_cache;

		Model::Scene* m_last_active_scene = nullptr;

		struct TreeViewState
		{
			VadonEditor::Model::Entity* clicked_entity = nullptr;
		};

		Internal(Core::Editor& editor)
			: m_instantiate_scene_dialog(editor)
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
				ViewSystem& view_system = editor.get_system<ViewSystem>();
				ViewModel& view_model = view_system.get_view_model();

				m_error_dialog.draw(dev_gui);

				Model::Scene* active_scene = view_model.get_active_scene();
				if (active_scene != nullptr)
				{
					Model::Entity* active_entity = view_model.get_active_entity();

					// TODO: dialog for creating entity?
					const bool add_entity = dev_gui.draw_button(m_add_entity_button);
					if (active_entity != nullptr)
					{
						if (dev_gui.draw_button(m_remove_entity_button) == true)
						{
							// TODO: prompt user for confirmation!
							active_scene->remove_entity(*active_entity);
							view_model.set_active_entity(nullptr);
						}
						else if (add_entity == true)
						{
							view_model.set_active_entity(active_scene->add_new_entity(*active_entity));
						}
					}
					else if (add_entity == true)
					{
						// Add to root
						view_model.set_active_entity(active_scene->add_new_entity(*active_scene->get_root()));
					}

					if (active_entity != nullptr)
					{
						if(dev_gui.draw_button(m_instantiate_scene_button) == true)
						{
							m_instantiate_scene_dialog.open();
						}

						if (m_instantiate_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
						{
							Model::Entity* instantiated_entity = active_scene->instantiate_sub_scene(m_instantiate_scene_dialog.get_selected_scene(), *active_entity);
							if (instantiated_entity != nullptr)
							{
								view_model.set_active_entity(instantiated_entity);
							}
							else
							{
								show_error("Scene Error", "Error instantiating sub-scene!");
							}
						}
					}

					// Make tree fill available content region
					m_tree_window.size = dev_gui.get_available_content_region();

					TreeViewState view_state;
					if (dev_gui.begin_child_window(m_tree_window) == true)
					{
						view_state = render_scene_tree(editor, dev_gui);
					}
					const bool window_clicked = dev_gui.is_window_hovered() && dev_gui.is_mouse_clicked(VadonApp::Platform::MouseButton::LEFT);
					dev_gui.end_child_window();

					if (view_state.clicked_entity != nullptr)
					{
						view_model.set_active_entity(view_state.clicked_entity);
					}
					else if (window_clicked == true)
					{
						view_model.set_active_entity(nullptr);
					}
				}
			}
			dev_gui.end_window();
		}

		void render_entity_node(Model::Entity* entity, VadonApp::UI::Developer::GUISystem& dev_gui, ViewModel& view_model, TreeViewState& tree_view_state)
		{
			constexpr VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;

			VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
			if (view_model.get_active_entity() == entity)
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
				std::format_to(std::back_inserter(m_tree_label_cache), "{} ({})", entity->get_name(), entity->get_sub_scene()->get_info().path.path);
				entity_name = m_tree_label_cache;
			}

			const bool node_open = dev_gui.push_tree_node(entity, entity_name, current_node_flags);
			if (dev_gui.is_item_clicked() && (dev_gui.is_item_toggled_open() == false))
			{
				tree_view_state.clicked_entity = entity;
			}

			if (node_open == true)
			{
				const Model::EntityList& child_entities = entity->get_children();
				for (Model::Entity* current_child : child_entities)
				{
					if (current_child->is_sub_scene_child() == false)
					{
						render_entity_node(current_child, dev_gui, view_model, tree_view_state);
					}
				}
				dev_gui.pop_tree_node();
			}
		}

		TreeViewState render_scene_tree(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			TreeViewState tree_view_state;

			ViewSystem& view_system = editor.get_system<ViewSystem>();
			ViewModel& view_model = view_system.get_view_model();

			render_entity_node(view_model.get_active_scene()->get_root(), dev_gui, view_model, tree_view_state);

			return tree_view_state;
		}

		void show_error(std::string_view title, std::string_view message)
		{
			m_error_dialog.set_title(title);
			m_error_dialog.set_message(message);
			m_error_dialog.open();
		}
	};

	SceneTreeWindow::~SceneTreeWindow() = default;

	SceneTreeWindow::SceneTreeWindow(Core::Editor& editor)
		: m_editor(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{

	}

	void SceneTreeWindow::draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		m_internal->draw(m_editor, dev_gui);
	}
}