#include <VadonEditor/View/Scene/Property/Property.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/View/Scene/Resource/Resource.hpp>

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
			m_input.input = std::get<Vadon::Utilities::Vector2>(model_property.value);
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
		void value_updated() override { m_input.input = std::get<Vadon::Utilities::Vector2>(m_property.value); }
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
			m_input.input = std::get<Vadon::Utilities::Vector3>(model_property.value);
		}

		bool render(VadonApp::UI::Developer::GUISystem& dev_gui) override
		{
			// FIXME: allow simple Vec3 input!
			if (dev_gui.draw_color3_picker(m_input) == true)
			{
				m_property.value = m_input.input;
				return true;
			}
			return false;
		}
	protected:
		void value_updated() override { m_input.input = std::get<Vadon::Utilities::Vector3>(m_property.value); }
	private:
		UI::Developer::InputFloat3 m_input;
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
		ResourcePropertyEditor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, bool use_handle)
			: PropertyEditor(model_property)
			, m_editor(editor)
			, m_select_resource_dialog(editor)
			, m_use_handle(use_handle)
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
				const Model::ResourceID selected_resource_id = m_select_resource_dialog.get_selected_resource();
				if (m_use_handle == true)
				{
					if (selected_resource_id.is_valid() == true)
					{
						VadonEditor::Model::ResourceSystem& resource_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
						Model::Resource* selected_resource = resource_system.get_resource(selected_resource_id);
						if (selected_resource->load() == false)
						{
							Vadon::Core::Logger::log_error("Resource property editor: failed to load selected resource!\n");
						}
						else
						{
							m_property.value = selected_resource->get_handle();
						}
					}
					else
					{
						m_property.value = Vadon::Scene::ResourceHandle();
					}
				}
				else
				{
					m_property.value = selected_resource_id;
				}

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
				if (m_use_handle == true)
				{
					m_property.value = Vadon::Scene::ResourceHandle();
				}
				else
				{
					m_property.value = Vadon::Scene::ResourceID();
				}
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
			// FIXME: deduplicate the code?
			if (m_use_handle == true)
			{
				Vadon::Scene::ResourceHandle resource_handle = std::get<Vadon::Scene::ResourceHandle>(m_property.value);

				if (resource_handle.is_valid() == true)
				{
					VadonEditor::Model::ResourceSystem& resource_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
					VadonEditor::Model::Resource* resource = resource_system.get_resource(resource_handle);

					const Model::ResourcePath path = resource->get_path();
					if (path.is_valid() == true)
					{
						m_label = path.path;
						return;
					}
				}
			}
			else
			{
				Vadon::Scene::ResourceID resource_id = std::get<Vadon::Scene::ResourceID>(m_property.value);
				if (resource_id.is_valid() == true)
				{
					VadonEditor::Model::ResourceSystem& resource_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
					const VadonEditor::Model::ResourceInfo resource_info = resource_system.get_database().find_resource_info(resource_id);
					VADON_ASSERT(resource_info.info.is_valid() == true, "Invalid resource ID!");

					if (resource_info.path.is_valid() == true)
					{
						m_label = resource_info.path.path;
						return;
					}
				}
			}

			m_label = "<NONE>";
		}

		Core::Editor& m_editor;
		bool m_use_handle;

		std::string m_header;
		std::string m_label;
		UI::Developer::Button m_select_resource_button;
		UI::Developer::Button m_clear_button;
		SelectResourceDialog m_select_resource_dialog; // FIXME: use a global instance instead of one per-property?
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
			case variant_type_list_index_v<Vadon::Utilities::Vector2>:
				return Instance(new Float2PropertyEditor(model_property));
			case variant_type_list_index_v<Vadon::Utilities::Vector3>:
				return Instance(new Float3PropertyEditor(model_property));
			case variant_type_list_index_v<std::string>:
				return Instance(new StringPropertyEditor(model_property));
			}
		}
		break;
		case Vadon::Utilities::ErasedDataType::RESOURCE_ID:
			return Instance(new ResourcePropertyEditor(editor, model_property, false));
		case Vadon::Utilities::ErasedDataType::RESOURCE_HANDLE:
			return Instance(new ResourcePropertyEditor(editor, model_property, true));
		}

		Vadon::Core::Logger::log_error("Property editor: no matching property editor available for data type!\n");
		return Instance();
	}
}