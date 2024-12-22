#ifndef VADONEDITOR_VIEW_SCENE_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_VIEW_SCENE_RESOURCE_RESOURCE_HPP
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/View/Scene/Property/Property.hpp>
#include <VadonEditor/UI/Developer/Widgets.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class SelectResourceDialog : public UI::Developer::Dialog
	{
	public:
		// TODO: filter resource type!
		SelectResourceDialog(Core::Editor& editor, std::string_view title = "Select Resource");

		void set_resource_type(Vadon::Utilities::TypeID resource_type) { m_resource_type = resource_type; }

		Model::ResourceID get_selected_resource() const;
	protected:
		Dialog::Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;

		Core::Editor& m_editor;
		Vadon::Utilities::TypeID m_resource_type;

		std::vector<Model::ResourceID> m_resource_list;

		UI::Developer::ListBox m_resource_list_box;
		UI::Developer::Button m_select_button;
		UI::Developer::Button m_cancel_button;
	};

	class ResourceEditor
	{
	private:
		ResourceEditor(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);

		void update_selected_resource(Model::Resource* resource);
		void update_labels(Model::Resource* resource);
		void reset_properties(Model::Resource* resource);

		Core::Editor& m_editor;

		Model::Resource* m_last_resource = nullptr;

		UI::Developer::Window m_window;
		std::vector<PropertyEditor::Instance> m_property_editors;

		std::string m_path;
		UI::Developer::Button m_save_button; // FIXME: save automatically? Or use some other logic?

		friend class MainWindow;
	};
}
#endif