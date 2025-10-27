#include <VadonEditor/View/Scene/Property/Property.hpp>

#include <VadonEditor/Core/Editor.hpp>

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
		IntPropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (read_only == false)
			{
				m_input.label = model_property.name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<int>(model_property.value);
			}
			else
			{
				// When read-only, use the label to store the read-only text
				m_input.label = std::format("{}: {}", model_property.name, std::get<int>(model_property.value));
			}
		}

		void value_updated() override { m_input.input = std::get<int>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (m_read_only == false)
			{
				if (dev_gui.draw_input_int(m_input) == true)
				{
					m_property.value = m_input.input;
					return true;
				}
			}
			else
			{
				dev_gui.add_text(m_input.label);
			}

			return false;
		}
	private:
		UI::Developer::InputInt m_input;
	};

	class FloatPropertyEditor final : public PropertyEditor
	{
	public:
		FloatPropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (read_only == false)
			{
				m_input.label = model_property.name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<float>(model_property.value);
			}
			else
			{
				// When read-only, use the label to store the read-only text
				m_input.label = std::format("{}: {}", model_property.name, std::get<float>(model_property.value));
			}
		}

		void value_updated() override { m_input.input = std::get<float>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (m_read_only == false)
			{
				if (dev_gui.draw_input_float(m_input) == true)
				{
					m_property.value = m_input.input;
					return true;
				}
			}
			else
			{
				dev_gui.add_text(m_input.label);
			}

			return false;
		}
	private:
		UI::Developer::InputFloat m_input;
	};

	class BoolPropertyEditor final : public PropertyEditor
	{
	public:
		BoolPropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (m_read_only == false)
			{
				m_checkbox.label = model_property.name; // TODO: parse name to create a more readable label?
				m_checkbox.checked = std::get<bool>(model_property.value);
			}
			else
			{
				// When read-only, use the label to store the read-only text
				const bool is_checked = std::get<bool>(model_property.value);
				m_checkbox.label = std::format("{}: {}", model_property.name, is_checked ? ICON_FA_CHECK : ICON_FA_X);
			}
		}

		void value_updated() override { m_checkbox.checked = std::get<bool>(m_property.value); }
	protected:

		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (m_read_only == false)
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
		Float2PropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (read_only == false)
			{
				m_input.label = model_property.name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<Vadon::Math::Vector2>(model_property.value);
			}
			else
			{
				const Vadon::Math::Vector2 value = std::get<Vadon::Math::Vector2>(model_property.value);
				m_input.label = std::format("{}: ( {} , {} )", model_property.name, value.x, value.y);
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
			else
			{
				dev_gui.add_text(m_input.label);
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
		Float3PropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (read_only == false)
			{
				m_input.label = model_property.name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<Vadon::Math::Vector3>(model_property.value);
			}
			else
			{
				const Vadon::Math::Vector3 value = std::get<Vadon::Math::Vector3>(model_property.value);
				m_input.label = std::format("{}: ( {} , {} , {} )", model_property.name, value.x, value.y, value.z);
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
		ColorPropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.value = std::get<Vadon::Math::ColorRGBA>(model_property.value);
		}

		void value_updated() override { m_input.value = std::get<Vadon::Math::ColorRGBA>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_color_edit(m_input, m_read_only) == true)
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
		StringPropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
		{
			if (read_only == false)
			{
				m_input.label = model_property.name; // TODO: parse name to create a more readable label?
				m_input.input = std::get<std::string>(model_property.value);
			}
			else
			{
				m_input.label = std::format("{}: \"{}\"", model_property.name, std::get<std::string>(model_property.value));
			}
		}

		void value_updated() override { m_input.input = std::get<std::string>(m_property.value); }
	protected:
		bool internal_render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (m_read_only == false)
			{
				if (dev_gui.draw_input_text(m_input) == true)
				{
					m_property.value = m_input.input;
					return true;
				}
			}
			else
			{
				dev_gui.add_text(m_input.label);
			}

			return false;
		}
	private:
		UI::Developer::InputText m_input;
	};

	struct ResourcePropertyEditor : public PropertyEditor
	{
	public:
		ResourcePropertyEditor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
			, m_editor(editor)
			, m_editor_widget(editor, true) // NOTE: the widget itself must be read-only, we're just showing the contents!
			, m_select_resource_dialog(editor)
		{
			m_header = model_property.name;

			if (read_only == false)
			{
				m_select_resource_button.label = "Select##resource_select_" + model_property.name;
				m_clear_button.label = "Clear##resource_clear_" + model_property.name;

				m_select_resource_dialog.set_resource_type(Vadon::Utilities::to_enum<Vadon::Utilities::TypeID>(model_property.data_type.id));
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
			if (m_select_resource_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
			{
				m_property.value = m_select_resource_dialog.get_selected_resource();
				update_resource();
				edited = true;
			}

			if (m_editor_widget.get_resource() != nullptr)
			{
				if (dev_gui.push_tree_node(m_label) == true)
				{
					if (m_read_only == false)
					{
						if (dev_gui.draw_button(m_select_resource_button) == true)
						{
							m_select_resource_dialog.open();
						}
						dev_gui.same_line();
						if (dev_gui.draw_button(m_clear_button) == true)
						{
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
				VadonEditor::Model::ResourceSystem& resource_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
				editor_resource = resource_system.get_resource(resource_id);
				VADON_ASSERT(editor_resource != nullptr, "Failed to create resource!");
				if (editor_resource->load() == false)
				{
					VADON_ERROR("Failed to load resource!");
				}

				std::string resource_path = editor_resource->get_path();
				if (resource_path.empty() == true)
				{
					resource_path = "UNSAVED";
				}

				const Vadon::Utilities::TypeInfo resource_type_info = Vadon::Utilities::TypeRegistry::get_type_info(editor_resource->get_info().type_id);
				m_label = std::format("{}: {} ({})", m_header, resource_path, resource_type_info.name);
			}
			else
			{
				m_label = m_header + ": <NONE>";
			}

			m_editor_widget.set_resource(editor_resource);
		}

		Core::Editor& m_editor;

		std::string m_header;
		UI::Developer::Button m_select_resource_button;
		UI::Developer::Button m_clear_button;

		ResourceEditorWidget m_editor_widget;
		std::string m_label;

		SelectResourceDialog m_select_resource_dialog; // FIXME: use a global instance instead of one per-property?
	};

	class ArrayPropertyEditor final : public PropertyEditor
	{
	public:
		ArrayPropertyEditor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, bool read_only)
			: PropertyEditor(model_property, read_only)
			, m_editor(editor)
		{
			m_array_child_window.string_id = model_property.name;
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
			const float slot_size = (dev_gui.calculate_text_size(m_add_button.label).y + dev_gui.calculate_text_size(m_property.name).y + gui_style.frame_padding.y * 2);
			m_array_child_window.size.y = slot_size * (std::max(m_array_elements.size(), 1ull) + 1);

			if (dev_gui.begin_child_window(m_array_child_window) == true)
			{
				dev_gui.add_text(m_property.name);

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
					if (m_read_only == false)
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
					Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
					array.data.erase(array.data.begin() + element_removed);

					rebuild_array_editors();
					array_edited = true;
				}

				if (m_read_only == false)
				{
					if (dev_gui.draw_button(m_add_button) == true)
					{
						Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
						array.data.push_back(Vadon::Utilities::get_erased_type_default_value(array.data_type));

						const size_t element_index = m_array_elements.size();
						m_array_elements.emplace_back(create_property_editor(m_editor, Vadon::Utilities::Property{ .name = std::format("{}", element_index), .data_type = array.data_type, .value = array.data.back() }, m_read_only));

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
			for (size_t index = 0; index < array.data.size(); ++index)
			{
				m_array_elements.emplace_back(create_property_editor(m_editor, Vadon::Utilities::Property{ .name = std::format("{}", index), .data_type = array.data_type, .value = array.data[index] }, m_read_only));
			}
		}

		Core::Editor& m_editor;
		
		std::vector<PropertyEditor::Instance> m_array_elements;

		UI::Developer::ChildWindow m_array_child_window;
		UI::Developer::Button m_add_button;
		UI::Developer::Button m_remove_button;
	};

	PropertyEditor::Instance PropertyEditor::create_property_editor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, bool read_only)
	{
		switch (model_property.data_type.type)
		{
		case Vadon::Utilities::ErasedDataType::TRIVIAL:
		{
			switch (model_property.data_type.id)
			{
			case variant_type_list_index_v<int>:
				return Instance(new IntPropertyEditor(model_property, read_only));
			case variant_type_list_index_v<float>:
				return Instance(new FloatPropertyEditor(model_property, read_only));
			case variant_type_list_index_v<bool>:
				return Instance(new BoolPropertyEditor(model_property, read_only));
			case variant_type_list_index_v<Vadon::Math::Vector2>:
				return Instance(new Float2PropertyEditor(model_property, read_only));
			case variant_type_list_index_v<Vadon::Math::Vector3>:
				return Instance(new Float3PropertyEditor(model_property, read_only));
			case variant_type_list_index_v<Vadon::Math::ColorRGBA>:
				return Instance(new ColorPropertyEditor(model_property, read_only));
			case variant_type_list_index_v<std::string>:
				return Instance(new StringPropertyEditor(model_property, read_only));
			}
		}
		break;
		case Vadon::Utilities::ErasedDataType::RESOURCE_ID:
			return Instance(new ResourcePropertyEditor(editor, model_property, read_only));
		case Vadon::Utilities::ErasedDataType::ARRAY:
			return Instance(new ArrayPropertyEditor(editor, model_property, read_only));
		}

		Vadon::Core::Logger::log_error("Property editor: no matching property editor available for data type!\n");
		return Instance();
	}
}