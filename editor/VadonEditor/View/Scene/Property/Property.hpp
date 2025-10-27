#ifndef VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#define VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
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
		bool render(UI::Developer::GUISystem& dev_gui)
		{
			const bool modified = internal_render(dev_gui);
			m_modified |= modified;

			return modified;
		}

		bool is_modified() const { return m_modified; }
		void clear_modified() { m_modified = false; }

		const Vadon::Utilities::Property& get_property() const { return m_property; }
		void set_value(const Vadon::Utilities::Variant& value) { m_property.value = value; clear_modified(); value_updated(); }

		static Instance create_property_editor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, bool read_only);
	protected:
		PropertyEditor(const Vadon::Utilities::Property& model_property, bool read_only)
			: m_property(model_property)
			, m_modified(false)
			, m_read_only(read_only)
		{}

		virtual bool internal_render(UI::Developer::GUISystem& dev_gui) = 0;

		// NOTE: subclasses need to implement how the UI elements should be updated based on the current value
		virtual void value_updated() = 0;

		// FIXME: could remove and retrieve data from derived classes?
		Vadon::Utilities::Property m_property;
		bool m_modified;
		bool m_read_only;
	};
}
#endif