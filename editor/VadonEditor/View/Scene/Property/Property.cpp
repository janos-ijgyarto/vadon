#include <VadonEditor/View/Scene/Property/Property.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/View/Scene/Resource/Resource.hpp>

#include <VadonApp/UI/Developer/IconsFontAwesome7.h>

#include <Vadon/Utilities/TypeInfo/Reflection/FunctionBind.hpp>

#include <algorithm>

namespace VadonEditor::View
{
	namespace
	{
		template<typename T>
		constexpr size_t variant_type_list_index_v = Vadon::Utilities::type_list_index_v<T, Vadon::Utilities::Variant>;
	}

	class IntPropertyEditor final : public PropertyEditor
	{
	public:
		IntPropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.input = std::get<int>(property_data.value);

			if (m_info.read_only == true)
			{
				m_input.flags |= UI::Developer::InputFlags::READ_ONLY;
			}
		}

		void value_updated() override { m_input.input = std::get<int>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_int(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::InputInt m_input;
	};

	class FloatPropertyEditor final : public PropertyEditor
	{
	public:
		FloatPropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.input = std::get<float>(property_data.value);

			if (m_info.read_only == true)
			{
				m_input.flags |= UI::Developer::InputFlags::READ_ONLY;
			}
		}

		void value_updated() override { m_input.input = std::get<float>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::InputFloat m_input;
	};

	class BoolPropertyEditor final : public PropertyEditor
	{
	public:
		BoolPropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			// TODO: simplify this once we have support for read-only checkbox?
			if (m_info.read_only == false)
			{
				m_checkbox.label = property_data.label;
				m_checkbox.checked = std::get<bool>(property_data.value);
			}
			else
			{
				// When read-only, use the label to store the read-only text
				const bool is_checked = std::get<bool>(m_property.value);
				m_checkbox.label = std::format("{}: {}", property_data.label, is_checked ? ICON_FA_CHECK : ICON_FA_X);
			}
		}

		void value_updated() override { m_checkbox.checked = std::get<bool>(m_property.value); }
	protected:

		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			// TODO: simplify this once we have support for read-only checkbox?
			if (m_info.read_only == false)
			{
				if (dev_gui.draw_checkbox(m_checkbox) == true)
				{
					m_property.value = m_checkbox.checked;
					return true;
				}
			}
			else
			{
				dev_gui.add_text(m_checkbox.label);
			}

			return false;
		}
	private:
		UI::Developer::Checkbox m_checkbox;
	};

	class Float2PropertyEditor final : public PropertyEditor
	{
	public:
		Float2PropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.input = std::get<Vadon::Math::Vector2>(property_data.value);

			if (m_info.read_only == true)
			{
				m_input.flags |= UI::Developer::InputFlags::READ_ONLY;
			}
		}

		void value_updated() override { m_input.input = std::get<Vadon::Math::Vector2>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float2(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::InputFloat2 m_input;
	};

	// FIXME: some way to do this with less boilerplate?
	class Float3PropertyEditor final : public PropertyEditor
	{
	public:
		Float3PropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.input = std::get<Vadon::Math::Vector3>(property_data.value);

			if (m_info.read_only == true)
			{
				m_input.flags |= UI::Developer::InputFlags::READ_ONLY;
			}
		}

		void value_updated() override { m_input.input = std::get<Vadon::Math::Vector3>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float3(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::InputFloat3 m_input;
	};

	class ColorPropertyEditor final : public PropertyEditor
	{
	public:
		ColorPropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.value = std::get<Vadon::Math::ColorRGBA>(property_data.value);
		}

		void value_updated() override { m_input.value = std::get<Vadon::Math::ColorRGBA>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_color_edit(m_input, m_info.read_only) == true)
			{
				m_property.value = m_input.value;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::ColorEdit m_input;
	};

	class StringPropertyEditor final : public PropertyEditor
	{
	public:
		StringPropertyEditor(const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
		{
			m_input.label = property_data.label;
			m_input.input = std::get<std::string>(property_data.value);

			if (m_info.read_only == true)
			{
				m_input.flags |= UI::Developer::InputFlags::READ_ONLY;
			}
		}

		void value_updated() override { m_input.input = std::get<std::string>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_text(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	private:
		UI::Developer::InputText m_input;
	};

	struct ResourcePropertyEditor : public PropertyEditor
	{
	public:
		ResourcePropertyEditor(Core::Editor& editor, const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
			, m_editor(editor)
			, m_editor_widget(editor)
			, m_create_resource_dialog(editor)
			, m_select_resource_dialog(editor)
		{
			m_header = property_data.label;

			m_editor_widget.set_read_only(m_info.read_only);

			if (m_info.read_only == false)
			{
				m_create_resource_button.label = "Create##resource_create_" + property_data.label;
				m_select_resource_button.label = "Select##resource_select_" + property_data.label;
				m_clear_button.label = "Clear##resource_clear_" + property_data.label;

				const Vadon::Utilities::TypeID resource_type = property_data.type.type_id;
				m_create_resource_dialog.set_resource_type(resource_type);
				m_select_resource_dialog.set_resource_type(resource_type);
			}

			update_resource();
		}

		void value_updated() override
		{
			update_resource();
		}
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			bool edited = false;
			// NOTE: do not allow embedded resources if no owner is provided!
			if (m_info.owner != nullptr)
			{
				if (m_create_resource_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
				{
					// TODO: check to make sure path is within project!		
					// First try to create the resource
					Model::ResourceSystem& editor_resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
					Model::Resource* new_resource = editor_resource_system.create_resource(m_create_resource_dialog.get_selected_resource_type());

					m_info.owner->add_embedded_resource(new_resource);

					m_property.value = new_resource->get_id();
					update_resource();
					edited = true;
				}
			}

			if (m_select_resource_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
			{
				Model::Resource* prev_resource = m_editor_widget.get_resource();
				if (prev_resource != nullptr)
				{
					if (prev_resource->is_embedded() == true)
					{
						Model::ResourceSystem& editor_resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
						editor_resource_system.remove_resource(prev_resource);
					}
				}

				m_property.value = m_select_resource_dialog.get_selected_resource();
				update_resource();
				edited = true;
			}

			if (m_editor_widget.get_resource() != nullptr)
			{
				if (dev_gui.push_tree_node(m_label) == true)
				{
					if (m_info.read_only == false)
					{
						// NOTE: do not allow embedded resources if no owner is provided!
						if (m_info.owner != nullptr)
						{
							if (dev_gui.draw_button(m_create_resource_button) == true)
							{
								m_create_resource_dialog.open();
							}
						}
						dev_gui.same_line();
						if (dev_gui.draw_button(m_select_resource_button) == true)
						{
							m_select_resource_dialog.open();
						}
						dev_gui.same_line();
						if (dev_gui.draw_button(m_clear_button) == true)
						{
							Model::Resource* prev_resource = m_editor_widget.get_resource();
							if (prev_resource->is_embedded() == true)
							{
								Model::ResourceSystem& editor_resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
								editor_resource_system.remove_resource(prev_resource);
							}

							m_property.value = Vadon::Scene::ResourceID();
							update_resource();
							edited = true;
						}
					}

					// FIXME: find a way to query from the editor how much space it needs!
					Vadon::Math::Vector2 editor_widget_size = dev_gui.get_available_content_region();
					editor_widget_size.y = std::max(editor_widget_size.y * 0.33f, 100.0f);

					if (m_editor_widget.draw(dev_gui, editor_widget_size) == true)
					{
						edited = true;
					}

					dev_gui.pop_tree_node();
				}
			}
			else
			{
				dev_gui.add_text_wrapped(m_label);
				if (m_info.read_only == false)
				{
					if (dev_gui.draw_button(m_create_resource_button) == true)
					{
						m_create_resource_dialog.open();
					}
					dev_gui.same_line();
					if (dev_gui.draw_button(m_select_resource_button) == true)
					{
						m_select_resource_dialog.open();
					}
				}
			}

			return edited;
		}
	private:
		void update_resource()
		{
			Vadon::Scene::ResourceID resource_id = std::get<Vadon::Scene::ResourceID>(m_property.value);
			Model::Resource* editor_resource = nullptr;
			if (resource_id.is_valid() == true)
			{
				VadonEditor::Model::ModelSystem& model_system = m_editor.get_system<VadonEditor::Model::ModelSystem>();
				VadonEditor::Model::ResourceSystem& resource_system = model_system.get_resource_system();
				editor_resource = resource_system.get_resource(resource_id);
				VADON_ASSERT(editor_resource != nullptr, "Failed to create resource!");
				if (editor_resource->load() == false)
				{
					VADON_ERROR("Failed to load resource!");
				}

				// Read-only, unless it's an embedded resource
				m_editor_widget.set_read_only((m_info.read_only == true) || (editor_resource->is_embedded() == false));

				std::string resource_path = editor_resource->get_path();
				if (resource_path.empty() == true)
				{
					if (editor_resource->is_embedded() == true)
					{
						resource_path = "EMBEDDED";
					}
					else
					{
						resource_path = "UNSAVED";
					}
				}

				VadonEditor::Core::MetadataRegistry& metadata_registry = m_editor.get_metadata_registry();
				const char* resource_type_name = metadata_registry.get_type_metadata(editor_resource->get_info().id, "name");
				if (resource_type_name == nullptr)
				{
					VADON_ERROR("Must provide resource metadata!");
				}

				m_label = std::format("{}: {} ({})", m_header, resource_path, resource_type_name);
			}
			else
			{
				m_label = m_header + ": <NONE>";
			}

			m_editor_widget.set_resource(editor_resource);
		}

		Core::Editor& m_editor;

		std::string m_header;
		UI::Developer::Button m_create_resource_button;
		UI::Developer::Button m_select_resource_button;
		UI::Developer::Button m_clear_button;

		ResourceEditorWidget m_editor_widget;
		std::string m_label;

		CreateResourceDialog m_create_resource_dialog;
		SelectResourceDialog m_select_resource_dialog; // FIXME: use a global instance instead of one per-property?
	};

	class ArrayPropertyEditor final : public PropertyEditor
	{
	public:
		ArrayPropertyEditor(Core::Editor& editor, const Model::Property& property_data, const PropertyEditorInfo& info)
			: PropertyEditor(property_data, info)
			, m_editor(editor)
		{
			m_array_child_window.string_id = property_data.label;
			m_array_child_window.border = true;

			m_add_button.label = "Add Element";
			m_remove_button.label = ICON_FA_TRASH;

			rebuild_array_editors();
		}

		void value_updated() override
		{
			// Array was modified externally, so we rebuild the editors
			// FIXME: this could be a problem if the array gets large, or if it messes up things during editing
			rebuild_array_editors();
		}
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			bool array_edited = false;

			m_array_child_window.size = dev_gui.get_available_content_region();

			// FIXME: make this properly align with the required vertical size
			const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();
			const float slot_size = (dev_gui.calculate_text_size(m_add_button.label).y + dev_gui.calculate_text_size(m_array_child_window.string_id).y + gui_style.frame_padding.y * 2);
			m_array_child_window.size.y = slot_size * (std::max(m_array_elements.size(), 1ull) + 1);

			if (dev_gui.begin_child_window(m_array_child_window) == true)
			{
				dev_gui.add_text(m_array_child_window.string_id);

				int32_t element_removed = -1;
				for (int array_index = 0; array_index < m_array_elements.size(); ++array_index)
				{
					const PropertyEditor::Instance& array_element_editor = m_array_elements[array_index];

					// Push ID (to ensure we don't collide with other elements
					dev_gui.push_id(array_index);
					if (array_element_editor->render(dev_gui) == true)
					{
						// TODO: make proper use of this and only update once explicitly requested (and only the modified elements)?
						array_element_editor->clear_modified();

						// Update the modified element in the data array
						Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
						array.data[array_index] = array_element_editor->get_property().value;

						array_edited = true;
					}
					if (m_info.read_only == false)
					{
						dev_gui.same_line();
						// TODO: id for each button
						if (dev_gui.draw_button(m_remove_button) == true)
						{
							element_removed = array_index;
						}
					}
					dev_gui.pop_id();
				}

				if (element_removed >= 0)
				{
					// We need to make sure to clean up any embedded resources that might be stored here
					remove_data_static(m_array_elements[element_removed]);

					Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
					array.data.erase(array.data.begin() + element_removed);

					rebuild_array_editors();
					array_edited = true;
				}

				if (m_info.read_only == false)
				{
					if (dev_gui.draw_button(m_add_button) == true)
					{
						Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
						array.data.push_back(Vadon::Utilities::get_erased_type_default_value(array.data_type));

						Model::Property array_element_data;
						const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Resource>();
						if (Vadon::Utilities::TypeRegistry::is_base_of(resource_type_id, m_property.type.type_id) == true)
						{
							array_element_data.type = { .category = Model::PropertyDataType::RESOURCE_ID, .type_id = m_property.type.type_id };
						}
						else
						{
							array_element_data.type = { .category = Model::PropertyDataType::TRIVIAL, .type_id = m_property.type.type_id };
						}

						const size_t element_index = m_array_elements.size();
						array_element_data.label = std::format("{}", element_index);

						array_element_data.value = array.data.back();

						m_array_elements.emplace_back(create_property_editor(m_editor, array_element_data, m_info));

						array_edited = true;
					}
				}
			}
			dev_gui.end_child_window();

			return array_edited;
		}
	private:
		void extract_array_data()
		{
			Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
			array.data.clear();

			for (const PropertyEditor::Instance& array_element_editor : m_array_elements)
			{
				array.data.push_back(array_element_editor->get_property().value);
			}
		}

		void rebuild_array_editors()
		{
			m_array_elements.clear();

			const Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
			const Vadon::Utilities::TypeUUID array_element_type_id = Vadon::Utilities::TypeRegistry::get_type_info(array.data_type).id;

			Model::Property property_data;

			const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Resource>();
			if (Vadon::Utilities::TypeRegistry::is_base_of(resource_type_id, m_property.type.type_id) == true)
			{
				property_data.type = { .category = Model::PropertyDataType::RESOURCE_ID, .type_id = m_property.type.type_id };
			}
			else
			{
				property_data.type = { .category = Model::PropertyDataType::TRIVIAL, .type_id = m_property.type.type_id };
			}

			for (size_t index = 0; index < array.data.size(); ++index)
			{
				property_data.label = std::format("{}", index);
				property_data.value = array.data[index];

				m_array_elements.emplace_back(create_property_editor(m_editor, property_data, m_info));
			}
		}

		Core::Editor& m_editor;
		
		std::vector<PropertyEditor::Instance> m_array_elements;

		UI::Developer::ChildWindow m_array_child_window;
		UI::Developer::Button m_add_button;
		UI::Developer::Button m_remove_button;
	};

	PropertyEditor::Instance PropertyEditor::create_property_editor(Core::Editor& editor, const Model::Property& property_data, const PropertyEditorInfo& info)
	{
		switch (property_data.type.category)
		{
		case Model::PropertyDataType::TRIVIAL:
		{
			// FIXME: find a more elegant way to do this!
			if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<int>())
			{
				return Instance(new IntPropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<float>())
			{
				return Instance(new FloatPropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<bool>())
			{
				return Instance(new BoolPropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Math::Vector2>())
			{
				return Instance(new Float2PropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Math::Vector3>())
			{
				return Instance(new Float3PropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Math::ColorRGBA>())
			{
				return Instance(new ColorPropertyEditor(property_data, info));
			}
			else if (property_data.type.type_id == Vadon::Utilities::TypeRegistry::get_type_id<std::string>())
			{
				return Instance(new StringPropertyEditor(property_data, info));
			}
		}
		break;
		case Model::PropertyDataType::RESOURCE_ID:
			return Instance(new ResourcePropertyEditor(editor, property_data, info));
		case Model::PropertyDataType::ARRAY:
			return Instance(new ArrayPropertyEditor(editor, property_data, info));
		}

		Vadon::Core::Logger::log_error("Property editor: no matching property editor available for data type!\n");
		return Instance();
	}
}