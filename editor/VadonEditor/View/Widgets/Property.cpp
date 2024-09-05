#include <VadonEditor/View/Widgets/Property.hpp>

#include <Vadon/Utilities/Data/Visitor.hpp>

namespace VadonEditor::View
{
	// FIXME: these all have basically the same API and behavior. Find some way to de-duplicate?
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

	PropertyEditor::Instance PropertyEditor::create_property_editor(const Vadon::Utilities::Property& model_property)
	{
		auto property_visitor = Vadon::Utilities::VisitorOverloadList{
			[&model_property](int)
			{
				return Instance(new IntPropertyEditor(model_property.name, model_property.value));
			},
			[&model_property](float)
			{
				return Instance(new FloatPropertyEditor(model_property.name, model_property.value));
			},
			[&model_property](bool)
			{
				return Instance(new BoolPropertyEditor(model_property.name, model_property.value));
			},
			[&model_property](Vadon::Utilities::Vector2)
			{
				return Instance(new Float2PropertyEditor(model_property.name, model_property.value));
			},
			[&model_property](Vadon::Utilities::Vector3)
			{
				return Instance(new Float3PropertyEditor(model_property.name, model_property.value));
			},
			[&model_property](std::string)
			{
				return Instance(new StringPropertyEditor(model_property.name, model_property.value));
			},
			[](auto) { 
				// TODO: error?
				return Instance();
			}
		};

		return std::visit(property_visitor, model_property.value);
	}
}