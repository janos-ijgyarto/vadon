#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Utilities/Data/Variant.hpp>
#include <Vadon/Utilities/Data/Visitor.hpp>

#include <format>

namespace VadonEditor::View
{
	namespace
	{
		// FIXME: move scene tree editing into dedicated subsystems!
		class PropertyEditor
		{
		public:
			virtual ~PropertyEditor() {}
			virtual bool render(VadonApp::UI::Developer::GUISystem& dev_gui) = 0;

			const std::string& get_name() const { return m_name; }
			const Vadon::Utilities::Variant& get_value() const { return m_value; }
			void set_value(const Vadon::Utilities::Variant& value) { m_value = value; value_updated(); }
		protected:
			PropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
				: m_name(property_name)
				, m_value(value)
			{}

			virtual void value_updated() = 0;

			std::string m_name;
			Vadon::Utilities::Variant m_value;
		};

		// FIXME: these all have basically the same API and behavior. Should these be de-duplicated?
		class IntPropertyEditor final : public PropertyEditor
		{
		public:
			IntPropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
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
			FloatPropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
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

		class BoolPropertyEditor final : public PropertyEditor
		{
		public:
			BoolPropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_checkbox.label = property_name; // TODO: parse name to create a more readable label?
				m_checkbox.checked = std::get<bool>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				if (dev_gui.draw_checkbox(m_checkbox) == true)
				{
					m_value = m_checkbox.checked;
					return true;
				}
				return false;
			}
		protected:
			void value_updated() override { m_checkbox.checked = std::get<bool>(m_value); }
		private:
			UI::Developer::Checkbox m_checkbox;
		};

		class Float2PropertyEditor final : public PropertyEditor
		{
		public:
			Float2PropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
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

		// FIXME: some way to do this with less boilerplate?
		class Float3PropertyEditor final : public PropertyEditor
		{
		public:
			Float3PropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
				: PropertyEditor(property_name, value)
			{
				m_input.label = property_name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<Vadon::Utilities::Vector3>(value);
			}

			bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
			{
				// FIXME: allow simple Vec3 input!
				if (dev_gui.draw_color3_picker(m_input) == true)
				{
					m_value = m_input.input;
					return true;
				}
				return false;
			}
		protected:
			void value_updated() override { m_input.input = std::get<Vadon::Utilities::Vector3>(m_value); }
		private:
			UI::Developer::InputFloat3 m_input;
		};

		class StringPropertyEditor final : public PropertyEditor
		{
		public:
			StringPropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
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

		struct ResourcePropertyEditor : public PropertyEditor
		{
		public:
			ResourcePropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
				: PropertyEditor(property_name, value)
			{
				// TODO!!!
			}
		private:
			std::vector<std::unique_ptr<PropertyEditor>> property_editors;
		};

		struct ErrorDialog
		{
			UI::Developer::Window window;
			UI::Developer::Button ok_button;

			std::string error_message;

			ErrorDialog()
			{
				window.title = "Error";
				ok_button.label = "Ok";
			}

			void show_error(VadonApp::UI::Developer::GUISystem& dev_gui, std::string_view message)
			{
				error_message = message;
				dev_gui.open_dialog(window.title);
			}

			// FIXME: store some kind of state to indicate whether dialog code should be run at all?
			void draw(VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.begin_modal_dialog(window) == true)
				{
					dev_gui.add_text(error_message);
					dev_gui.end_dialog();
				}
			}
		};

		struct NewSceneDialog
		{
			UI::Developer::Window window;

			UI::Developer::Button new_scene_button;
			UI::Developer::Button add_button;

			UI::Developer::InputText scene_name;

			NewSceneDialog()
			{
				window.title = "Create New Scene";

				new_scene_button.label = "New scene";
				add_button.label = "Create";

				scene_name.label = "Scene name";
			}

			bool draw(VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.draw_button(new_scene_button) == true)
				{
					scene_name.input.clear();
					dev_gui.open_dialog(window.title);
				}

				bool scene_created = false;
				if (dev_gui.begin_modal_dialog(window) == true)
				{
					dev_gui.draw_input_text(scene_name);

					if (has_valid_name() == false)
					{
						dev_gui.begin_disabled();
					}
					if (dev_gui.draw_button(add_button) == true)
					{
						scene_created = true;
						dev_gui.close_current_dialog();
					}
					if (has_valid_name() == false)
					{
						dev_gui.end_disabled();
					}

					dev_gui.end_dialog();
				}

				return scene_created;
			}

			const std::string& get_new_scene_name() const
			{
				return scene_name.input;
			}

			bool has_valid_name() const
			{
				return scene_name.input.empty() == false;
			}
		};

		struct LoadSceneDialog
		{
			UI::Developer::Window window;

			UI::Developer::ListBox scene_item_list;
			VadonEditor::Model::SceneList scene_list;

			UI::Developer::Button load_scene_button;
			UI::Developer::Button load_button;

			LoadSceneDialog()
			{
				window.title = "Load Scene";

				load_scene_button.label = "Load scene";
				load_button.label = "Load";

				scene_item_list.label = "Scenes";
			}

			bool draw(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.draw_button(load_scene_button) == true)
				{
					// Rebuild scene list
					Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
					Model::SceneTree& scene_tree = model.get_scene_tree();

					scene_item_list.items.clear();
					scene_list = scene_tree.get_scene_list();

					for (const Model::Scene& current_scene : scene_list)
					{
						scene_item_list.items.push_back(current_scene.path);
					}

					scene_item_list.selected_item = 0;

					dev_gui.open_dialog(window.title);
				}

				bool scene_loaded = false;
				if (dev_gui.begin_modal_dialog(window) == true)
				{
					dev_gui.draw_list_box(scene_item_list);
					if (has_valid_option() == false)
					{
						dev_gui.begin_disabled();
					}
					if (dev_gui.draw_button(load_button) == true)
					{
						scene_loaded = true;
						dev_gui.close_current_dialog();
					}
					if (has_valid_option() == false)
					{
						dev_gui.end_disabled();
					}

					dev_gui.end_dialog();
				}

				return scene_loaded;
			}

			Vadon::Scene::ResourceID get_loaded_scene() const
			{
				return scene_list[scene_item_list.selected_item].scene_id;
			}

			bool has_valid_option() const
			{
				return scene_list.empty() == false;
			}
		};

		struct InstantiateSceneDialog : public LoadSceneDialog
		{
			InstantiateSceneDialog()
			{
				window.title = "Instantiate Sub-Scene";

				load_scene_button.label = "Instantiate sub-scene";
				load_button.label = "Instantiate";

				scene_item_list.label = "Scenes";
			}
		};

		struct AddComponentDialog
		{
			UI::Developer::Window window;
			
			UI::Developer::Button add_button;
			UI::Developer::Button ok_button;

			UI::Developer::ListBox component_type_list;
			Vadon::ECS::ComponentIDList component_type_ids;

			AddComponentDialog()
			{
				window.title = "Add Component";

				add_button.label = "Add component";
				ok_button.label = "OK";

				component_type_list.label = "Component types";
			}

			bool draw(Model::Entity& entity, VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.draw_button(add_button) == true)
				{
					// Rebuild component type list
					component_type_list.items.clear();
					component_type_ids.clear();

					const Vadon::Utilities::TypeInfoList component_info_list = entity.get_available_component_list();
					for (const Vadon::Utilities::TypeInfo& current_component_type_info : component_info_list)
					{
						component_type_list.items.push_back(current_component_type_info.name);
						component_type_ids.push_back(current_component_type_info.id);
					}

					dev_gui.open_dialog(window.title);
				}

				bool component_added = false;
				if (dev_gui.begin_modal_dialog(window) == true)
				{
					dev_gui.draw_list_box(component_type_list);

					if (has_valid_option() == false)
					{
						dev_gui.begin_disabled();
					}
					if (dev_gui.draw_button(ok_button) == true)
					{
						component_added = true;
						dev_gui.close_current_dialog();
					}
					if (has_valid_option() == false)
					{
						dev_gui.end_disabled();
					}

					dev_gui.end_dialog();
				}

				return component_added;
			}

			Vadon::ECS::ComponentID get_selected_component_type() const
			{
				return component_type_ids[component_type_list.selected_item];
			}

			bool has_valid_option() const
			{
				return component_type_ids.empty() == false;
			}
		};

		struct ComponentEditor
		{
			Vadon::ECS::ComponentID type_id;
			std::string name;

			std::vector<std::unique_ptr<PropertyEditor>> property_editors;

			VadonApp::UI::Developer::GUISystem::ID property_editors_id = 0;

			VadonApp::UI::Developer::Button remove_button;

			ComponentEditor()
			{
				remove_button.label = "Remove component";
			}

			void initialize(Model::Entity& entity)
			{
				const VadonEditor::Model::Component component_data = entity.get_component_data(type_id);
				name = component_data.name;

				for (const Vadon::Utilities::Property& current_property : component_data.properties)
				{
					auto property_visitor = Vadon::Utilities::VisitorOverloadList{
						[this, &current_property](int)
						{
							property_editors.emplace_back(new IntPropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](float)
						{
							property_editors.emplace_back(new FloatPropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](bool)
						{
							property_editors.emplace_back(new BoolPropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](Vadon::Utilities::Vector2)
						{
							property_editors.emplace_back(new Float2PropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](Vadon::Utilities::Vector3)
						{
							property_editors.emplace_back(new Float3PropertyEditor(current_property.name, current_property.value));
						},
						[this, &current_property](std::string)
						{
							property_editors.emplace_back(new StringPropertyEditor(current_property.name, current_property.value));
						},
						[](auto) { /* TODO: error? */ }
					};

					std::visit(property_visitor, current_property.value);
				}
			}

			bool draw(VadonApp::UI::Developer::GUISystem& dev_gui, Model::Entity& entity)
			{
				// TODO: context menus and/or buttons for adding/removing components
				bool removed = false;
				if (dev_gui.push_tree_node(name) == true)
				{
					for (auto& current_property : property_editors)
					{
						if (current_property->render(dev_gui) == true)
						{
							// Property edited, send update to scene tree and update the editor
							// FIXME: optimize this somehow?
							entity.edit_component_property(type_id, current_property->get_name(), current_property->get_value());
							current_property->set_value(entity.get_component_property(type_id, current_property->get_name()));
						}
					}
					if (dev_gui.draw_button(remove_button) == true)
					{
						entity.remove_component(type_id);
						removed = true;
					}
					dev_gui.pop_tree_node();					
				}
				return removed;
			}
		};

		struct EntityEditor
		{
			Model::Entity* selected_entity = nullptr;
			std::string selected_entity_path;

			UI::Developer::InputText entity_name_input;
			std::vector<ComponentEditor> component_editors;

			UI::Developer::ChildWindow component_tree; // Using tree to make components collapsable

			AddComponentDialog add_component_dialog;

			EntityEditor()
			{
				entity_name_input.label = "Entity name";

				component_tree.id = "ComponentTree";
				component_tree.border = true;
			}

			void draw(VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (selected_entity == nullptr)
				{
					return;
				}

				dev_gui.push_id(selected_entity->get_id());
				if (dev_gui.draw_input_text(entity_name_input) == true)
				{
					selected_entity->set_name(entity_name_input.input);
					update_name();
				}
				dev_gui.add_text(selected_entity_path);

				if (add_component_dialog.draw(*selected_entity, dev_gui) == true)
				{
					selected_entity->add_component(add_component_dialog.get_selected_component_type());
					reset_components();
				}

				if (component_editors.empty() == false)
				{
					// TODO: revise this so we show the property editors for only one component at a time
					// and reset the view whenever a component is added/removed
					bool component_removed = false;
					if (dev_gui.begin_child_window(component_tree) == true)
					{
						for (ComponentEditor& current_component : component_editors)
						{
							component_removed |= current_component.draw(dev_gui, *selected_entity);
						}
					}
					dev_gui.end_child_window();
					if (component_removed == true)
					{
						reset_components();
					}
				}
				dev_gui.pop_id();
			}

			void set_selected_entity(Model::Entity* entity)
			{
				if (selected_entity == entity)
				{
					return;
				}

				selected_entity = entity;
				if (selected_entity == nullptr)
				{
					return;
				}

				update_name();
				reset_components();
			}

			void update_name()
			{
				entity_name_input.input = selected_entity->get_name();
				selected_entity_path = "Path: " + selected_entity->get_path();
			}

			void reset_components()
			{
				component_editors.clear();

				const Vadon::ECS::ComponentIDList component_id_list = selected_entity->get_component_types();
				for (Vadon::ECS::ComponentID current_component_id : component_id_list)
				{
					ComponentEditor& current_component_editor = component_editors.emplace_back();
					current_component_editor.type_id = current_component_id;

					current_component_editor.initialize(*selected_entity);
				}
			}
		};

		class SceneEditor
		{
		public:
			struct SceneTreeViewState
			{
				VadonEditor::Model::Entity* clicked_entity = nullptr;
			};

			SceneEditor()
			{
				m_window.title = "Entity Editor";
				m_window.open = false;

				m_tree_window.id = "Tree";
				m_tree_window.size = Vadon::Utilities::Vector2(400, 300);
				m_tree_window.border = true;

				m_add_entity_button.label = "Add entity";
				m_remove_entity_button.label = "Remove entity";

				m_save_scene_button.label = "Save scene";
			}

			void render(Core::Editor& editor)
			{
				VadonApp::Core::Application& engine_app = editor.get_engine_app();
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				if (dev_gui.begin_window(m_window) == true)
				{
					if (dev_gui.begin_main_menu_bar() == true)
					{
						if (dev_gui.begin_menu("Scene") == true)
						{
							if (dev_gui.add_menu_item(VadonApp::UI::Developer::MenuItem{ .label = "New Scene" }) == true)
							{
								// TODO
							}
							if (dev_gui.add_menu_item(VadonApp::UI::Developer::MenuItem{ .label = "Load Scene" }) == true)
							{
								// TODO
							}
							dev_gui.end_menu();
						}

						if (dev_gui.begin_menu("Project") == true)
						{
							if (dev_gui.add_menu_item(VadonApp::UI::Developer::MenuItem{ .label = "Project Settings" }) == true)
							{
								// TODO
							}
							if (dev_gui.add_menu_item(VadonApp::UI::Developer::MenuItem{ .label = "Close Project" }) == true)
							{
								// TODO
							}
							dev_gui.end_menu();
						}

						dev_gui.end_main_menu_bar();
					}

					Model::ModelSystem& model = editor.get_system<Model::ModelSystem>();
					Model::SceneTree& scene_tree = model.get_scene_tree();

					m_error_dialog.draw(dev_gui);

					if (m_new_scene_dialog.draw(dev_gui) == true)
					{
						if (scene_tree.new_scene(m_new_scene_dialog.get_new_scene_name()) == false)
						{
							m_error_dialog.show_error(dev_gui, "Error creating scene!");
						}
					}
					if (m_load_scene_dialog.draw(editor, dev_gui) == true)
					{
						if (scene_tree.load_scene(m_load_scene_dialog.get_loaded_scene()) == false)
						{
							m_error_dialog.show_error(dev_gui, "Error loading scene!");
						}
					}
					if (scene_tree.get_current_scene().is_valid() == true)
					{
						if (m_entity_editor.selected_entity != nullptr)
						{
							if (m_instantiate_scene_dialog.draw(editor, dev_gui) == true)
							{
								if (scene_tree.instantiate_sub_scene(m_entity_editor.selected_entity, m_instantiate_scene_dialog.get_loaded_scene()) == false)
								{
									m_error_dialog.show_error(dev_gui, "Error instantiating sub-scene!");
								}
							}
						}

						const bool add_entity = dev_gui.draw_button(m_add_entity_button);
						if (m_entity_editor.selected_entity != nullptr)
						{
							if (dev_gui.draw_button(m_remove_entity_button) == true)
							{
								// TODO: prompt user for confirmation!
								scene_tree.remove_entity(m_entity_editor.selected_entity);
								m_entity_editor.set_selected_entity(nullptr);
							}
							else if (add_entity == true)
							{
								scene_tree.add_entity(m_entity_editor.selected_entity);
							}
						}
						else if (add_entity == true)
						{
							scene_tree.add_entity();
						}

						SceneTreeViewState view_state;
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

						m_entity_editor.draw(dev_gui);

						Model::Entity* scene_root = scene_tree.get_root();
						if (scene_root != nullptr)
						{
							if (dev_gui.draw_button(m_save_scene_button) == true)
							{
								scene_tree.save_scene();
							}
						}
					}
				}
				dev_gui.end_window();
			}
		private:
			void render_entity_node(Model::Entity* entity, VadonApp::UI::Developer::GUISystem& dev_gui, SceneTreeViewState& view_state)
			{
				constexpr VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;

				VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
				if (m_entity_editor.selected_entity == entity)
				{
					current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::SELECTED;
				}
				if (entity->has_visible_children() == false)
				{
					current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::LEAF;
				}

				const bool node_open = dev_gui.push_tree_node(entity, entity->get_name(), current_node_flags);
				if (dev_gui.is_item_clicked() && (dev_gui.is_item_toggled_open() == false))
				{
					view_state.clicked_entity = entity;
				}

				if (node_open == true)
				{
					const Model::EntityList& child_entities = entity->get_children();
					for (Model::Entity* current_child : child_entities)
					{
						if (current_child->is_scene_child() == false)
						{
							render_entity_node(current_child, dev_gui, view_state);
						}
					}
					dev_gui.pop_tree_node();
				}
			}

			SceneTreeViewState render_scene_tree(Core::Editor& editor, VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				SceneTreeViewState tree_view_state;

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

			UI::Developer::Window m_window;

			ErrorDialog m_error_dialog;

			NewSceneDialog m_new_scene_dialog;
			LoadSceneDialog m_load_scene_dialog;
			InstantiateSceneDialog m_instantiate_scene_dialog;
			UI::Developer::Button m_save_scene_button;

			Model::Entity* m_scene_root = nullptr;

			UI::Developer::Button m_add_entity_button;
			UI::Developer::Button m_remove_entity_button;

			UI::Developer::ChildWindow m_tree_window;

			EntityEditor m_entity_editor;
		};

		// FIXME: move to its own subsystem!
		class ECSDebugView
		{
		public:
			ECSDebugView()
			{
				m_window.title = "ECS Debug View";
				m_window.open = false;

				m_entity_list_box.label = "Entities";
				m_update_entity_list_button.label = "Update";
			}

			void render(Core::Editor& editor)
			{
				VadonApp::Core::Application& engine_app = editor.get_engine_app();
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				if (dev_gui.begin_window(m_window) == true)
				{
					dev_gui.draw_list_box(m_entity_list_box);

					if (dev_gui.draw_button(m_update_entity_list_button) == true)
					{
						update_entity_list(editor);
					}
				}
				dev_gui.end_window();
			}
		private:
			void update_entity_list(Core::Editor& editor)
			{
				m_entity_list_box.items.clear();

				Vadon::ECS::EntityManager& entity_manager = editor.get_system<Model::ModelSystem>().get_ecs_world().get_entity_manager();
				m_entities = entity_manager.get_active_entities();

				for (const Vadon::ECS::EntityHandle& current_entity : m_entities)
				{
					const std::string entity_label = std::format("{} ({}_{})", entity_manager.get_entity_name(current_entity), current_entity.handle.index, current_entity.handle.generation);
					m_entity_list_box.items.push_back(entity_label);
				}
			}

			UI::Developer::Window m_window;

			UI::Developer::ListBox m_entity_list_box;
			UI::Developer::Button m_update_entity_list_button;

			Vadon::ECS::EntityList m_entities;
		};
	}

	struct ViewSystem::Internal
	{
		Core::Editor& m_editor;

		SceneEditor m_scene_editor;
		ECSDebugView m_ecs_debug_view;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{
		}

		bool initialize()
		{
			m_editor.get_system<UI::UISystem>().register_ui_element([this](Core::Editor& editor) { 
				m_scene_editor.render(editor); 
				m_ecs_debug_view.render(editor);
				});
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