#include <VadonEditor/View/Scene/Property/Property.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/View/Scene/Resource/Resource.hpp>

#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <Vadon/Utilities/TypeInfo/Reflection/FunctionBind.hpp>

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
		IntPropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.input = std::get<int>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_int(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<int>(m_property.value); }
	private:
		UI::Developer::InputInt m_input;
	};

	class FloatPropertyEditor final : public PropertyEditor
	{
	public:
		FloatPropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.input = std::get<float>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<float>(m_property.value); }
	private:
		UI::Developer::InputFloat m_input;
	};

	class BoolPropertyEditor final : public PropertyEditor
	{
	public:
		BoolPropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_checkbox.label = model_property.name; // TODO: parse name to create a more readable label?
			m_checkbox.checked = std::get<bool>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_checkbox(m_checkbox) == true)
			{
				m_property.value = m_checkbox.checked;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_checkbox.checked = std::get<bool>(m_property.value); }
	private:
		UI::Developer::Checkbox m_checkbox;
	};

	class Float2PropertyEditor final : public PropertyEditor
	{
	public:
		Float2PropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.input = std::get<Vadon::Math::Vector2>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float2(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<Vadon::Math::Vector2>(m_property.value); }
	private:
		UI::Developer::InputFloat2 m_input;
	};

	// FIXME: some way to do this with less boilerplate?
	class Float3PropertyEditor final : public PropertyEditor
	{
	public:
		Float3PropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.input = std::get<Vadon::Math::Vector3>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_float3(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<Vadon::Math::Vector3>(m_property.value); }
	private:
		UI::Developer::InputFloat3 m_input;
	};

	class ColorPropertyEditor final : public PropertyEditor
	{
	public:
		ColorPropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.value = std::get<Vadon::Math::ColorRGBA>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_color_edit(m_input) == true)
			{
				m_property.value = m_input.value;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.value = std::get<Vadon::Math::ColorRGBA>(m_property.value); }
	private:
		UI::Developer::ColorEdit m_input;
	};

	class StringPropertyEditor final : public PropertyEditor
	{
	public:
		StringPropertyEditor(const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
		{
			m_input.label = model_property.name; // TODO: parse name to create a more readable label?
			m_input.input = std::get<std::string>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			if (dev_gui.draw_input_text(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}

			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<std::string>(m_property.value); }
	private:
		UI::Developer::InputText m_input;
	};

	struct ResourcePropertyEditor : public PropertyEditor
	{
	public:
		ResourcePropertyEditor(Core::Editor& editor, const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
			, m_editor(editor)
			, m_select_resource_dialog(editor)
		{
			update_resource_reference();

			m_header = model_property.name + ":";

			m_select_resource_button.label = "Select##resource_select_" + model_property.name;
			m_clear_button.label = "Clear##resource_clear_" + model_property.name;

			m_select_resource_dialog.set_resource_type(Vadon::Utilities::to_enum<Vadon::Utilities::TypeID>(model_property.data_type.id));
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			bool edited = false;
			if (m_select_resource_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
			{
				m_property.value = m_select_resource_dialog.get_selected_resource();
				edited = true;
			}

			dev_gui.add_text(m_header);
			dev_gui.same_line();
			dev_gui.add_text(m_label);
			dev_gui.same_line();
			if (dev_gui.draw_button(m_select_resource_button) == true)
			{
				m_select_resource_dialog.open();
			}
			dev_gui.same_line();
			if (dev_gui.draw_button(m_clear_button) == true)
			{
				m_property.value = Vadon::Scene::ResourceID();
				edited = true;
			}

			return edited;
		}
	protected:
		void value_updated() override 
		{
			update_resource_reference();
		}
	private:
		void update_resource_reference()
		{
			Vadon::Scene::ResourceID resource_id = std::get<Vadon::Scene::ResourceID>(m_property.value);
			if (resource_id.is_valid() == true)
			{
				VadonEditor::Model::ResourceSystem& resource_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
				const VadonEditor::Model::ResourceInfo* resource_info = resource_system.get_database().find_resource_info(resource_id);
				VADON_ASSERT(resource_info != nullptr, "Invalid resource ID!");

				if (resource_info->path.empty() == false)
				{
					m_label = resource_info->path;
					return;
				}
			}

			m_label = "<NONE>";
		}

		Core::Editor& m_editor;

		std::string m_header;
		std::string m_label;
		UI::Developer::Button m_select_resource_button;
		UI::Developer::Button m_clear_button;
		SelectResourceDialog m_select_resource_dialog; // FIXME: use a global instance instead of one per-property?
	};

	class ArrayPropertyEditor final : public PropertyEditor
	{
	public:
		ArrayPropertyEditor(Core::Editor& editor, const Vadon::Utilities::Property& model_property)
			: PropertyEditor(model_property)
			, m_editor(editor)
		{
			m_array_child_window.id = model_property.name;
			m_array_child_window.border = true;

			m_add_button.label = "Add Element";
			m_remove_button.label = ICON_FA_TRASH;

			rebuild_array_editors();
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			bool array_edited = false;
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
						// Update the modified element in the data array
						Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
						array.data[array_index] = array_element_editor->get_property().value;

						// Notify the element editor
						array_element_editor->value_updated();

						array_edited = true;
					}
					dev_gui.same_line();
					// TODO: id for each button
					if (dev_gui.draw_button(m_remove_button) == true)
					{
						element_removed = array_index;
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

				if (dev_gui.draw_button(m_add_button) == true)
				{
					Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(m_property.value);
					array.data.push_back(Vadon::Utilities::get_erased_type_default_value(array.data_type));

					const size_t element_index = m_array_elements.size();
					m_array_elements.emplace_back(create_property_editor(m_editor, Vadon::Utilities::Property{ .name = std::format("{}", element_index), .data_type = array.data_type, .value = array.data.back() }));

					array_edited = true;
				}
			}
			dev_gui.end_child_window();

			if (array_edited == true)
			{
				// Set the synced flag, as we will prompt the parent widget to send an update signal
				m_array_editor_synced = true;
			}

			return array_edited;
		}
	protected:
		void value_updated() override 
		{
			// Check whether the editor was already synced
			if (m_array_editor_synced == false)
			{
				// Array was modified externally, so we rebuild the editors
				rebuild_array_editors();
			}

			// Reset flag
			m_array_editor_synced = false;
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
				m_array_elements.emplace_back(create_property_editor(m_editor, Vadon::Utilities::Property{ .name = std::format("{}", index), .data_type = array.data_type, .value = array.data[index] }));
			}
		}

		Core::Editor& m_editor;
		
		std::vector<PropertyEditor::Instance> m_array_elements;
		bool m_array_editor_synced = false; // NOTE: this is used to avoid redundantly resetting the widgets

		UI::Developer::ChildWindow m_array_child_window;
		UI::Developer::Button m_add_button;
		UI::Developer::Button m_remove_button;
	};

	PropertyEditor::Instance PropertyEditor::create_property_editor(Core::Editor& editor, const Vadon::Utilities::Property& model_property)
	{
		switch (model_property.data_type.type)
		{
		case Vadon::Utilities::ErasedDataType::TRIVIAL:
		{
			switch (model_property.data_type.id)
			{
			case variant_type_list_index_v<int>:
				return Instance(new IntPropertyEditor(model_property));
			case variant_type_list_index_v<float>:
				return Instance(new FloatPropertyEditor(model_property));
			case variant_type_list_index_v<bool>:
				return Instance(new BoolPropertyEditor(model_property));
			case variant_type_list_index_v<Vadon::Math::Vector2>:
				return Instance(new Float2PropertyEditor(model_property));
			case variant_type_list_index_v<Vadon::Math::Vector3>:
				return Instance(new Float3PropertyEditor(model_property));
			case variant_type_list_index_v<Vadon::Math::ColorRGBA>:
				return Instance(new ColorPropertyEditor(model_property));
			case variant_type_list_index_v<std::string>:
				return Instance(new StringPropertyEditor(model_property));
			}
		}
		break;
		case Vadon::Utilities::ErasedDataType::RESOURCE_ID:
			return Instance(new ResourcePropertyEditor(editor, model_property));
		case Vadon::Utilities::ErasedDataType::ARRAY:
			return Instance(new ArrayPropertyEditor(editor, model_property));
		}

		Vadon::Core::Logger::log_error("Property editor: no matching property editor available for data type!\n");
		return Instance();
	}
}