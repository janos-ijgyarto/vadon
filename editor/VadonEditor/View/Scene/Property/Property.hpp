#ifndef VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#define VADONEDITOR_VIEW_SCENE_PROPERTY_PROPERTY_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::View
{
	struct PropertyEditorInfo
	{
		VadonEditor::Model::Resource* owner = nullptr;
		bool read_only = false;
	};

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

		static Instance create_property_editor(Core::Editor& editor, const Vadon::Utilities::Property& model_property, const PropertyEditorInfo& info);
	protected:
		PropertyEditor(const Vadon::Utilities::Property& model_property, const PropertyEditorInfo& info)
			: m_property(model_property)
			, m_info(info)
			, m_modified(false)
		{
			VADON_ASSERT(info.owner != nullptr, "Must set a valid owner!");
		}

		virtual bool internal_render(UI::Developer::GUISystem& dev_gui) = 0;

		// NOTE: subclasses need to implement how the UI elements should be updated based on the current value
		virtual void value_updated() = 0;

		// NOTE: this is needed for resources, if an embedded resource is removed from a container then we need to make sure to remove the resource
		virtual void remove_data() {}

		// FIXME: this is a clumsy workaround because protected member function cannot be accessed from derived instance
		static void remove_data_static(Instance& instance) { instance->remove_data(); }

		// FIXME: could remove and retrieve data from derived classes?
		Vadon::Utilities::Property m_property;
		PropertyEditorInfo m_info;

		bool m_modified;
	};
}
#endif