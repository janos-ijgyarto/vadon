#ifndef VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#define VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Property.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class PropertyEditor
	{
	public:
		using Instance = std::unique_ptr<PropertyEditor>;

		virtual ~PropertyEditor() {}
		virtual bool render(UI::Developer::GUISystem& dev_gui) = 0;

		const std::string& get_name() const { return m_property.name; }
		const Vadon::Utilities::Variant& get_value() const { return m_property.value; }
		void set_value(const Vadon::Utilities::Variant& value) { m_property.value = value; value_updated(); }

		static Instance create_property_editor(Core::Editor& editor, const Vadon::Utilities::Property& model_property);
	protected:
		PropertyEditor(const Vadon::Utilities::Property& model_property)
			: m_property(model_property)
		{}

		virtual void value_updated() = 0;

		// FIXME: could remove and retrieve data from derived classes?
		Vadon::Utilities::Property m_property;
	};
}
#endif