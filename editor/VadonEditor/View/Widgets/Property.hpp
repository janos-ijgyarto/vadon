#ifndef VADONEDITOR_VIEW_WIDGETS_PROPERTY_HPP
#define VADONEDITOR_VIEW_WIDGETS_PROPERTY_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Property.hpp>
namespace VadonEditor::View
{
	class PropertyEditor
	{
	public:
		using Instance = std::unique_ptr<PropertyEditor>;

		virtual ~PropertyEditor() {}
		virtual bool render(UI::Developer::GUISystem& dev_gui) = 0;

		const std::string& get_name() const { return m_name; }
		const Vadon::Utilities::Variant& get_value() const { return m_value; }
		void set_value(const Vadon::Utilities::Variant& value) { m_value = value; value_updated(); }

		static Instance create_property_editor(const Vadon::Utilities::Property& model_property);
	protected:
		PropertyEditor(std::string_view property_name, const Vadon::Utilities::Variant& value)
			: m_name(property_name)
			, m_value(value)
		{}

		virtual void value_updated() = 0;

		std::string m_name;
		Vadon::Utilities::Variant m_value;
	};
}
#endif