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
	class CreateResourceDialog : public UI::Developer::Dialog
	{
	public:
		CreateResourceDialog(Core::Editor& editor);

		void set_resource_type(Vadon::Utilities::TypeID resource_type);

		Vadon::Utilities::TypeID get_selected_resource_type() const;
	protected:
		void on_open();
		Result internal_draw(UI::Developer::GUISystem& dev_gui) override;
	private:
		void reset_type_list();

		Core::Editor& m_editor;
		Vadon::Utilities::TypeID m_resource_type;

		// FIXME: use a tree view so we can see and search subclass hierarchies?
		std::vector<Vadon::Utilities::TypeID> m_resource_types;
		UI::Developer::ComboBox m_resource_type_combo;

		UI::Developer::Button m_accept_button;
		UI::Developer::Button m_cancel_button;
	};

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

	class ResourceEditorWidget
	{
	public:
		ResourceEditorWidget(Core::Editor& editor);

		void set_read_only(bool read_only) { m_read_only = read_only; reset_properties(m_resource); }

		void set_resource(Model::Resource* resource);
		Model::Resource* get_resource() const { return m_resource; }

		bool draw(UI::Developer::GUISystem& dev_gui, const Vadon::Math::Vector2& size = { 0, 0 }); // Returning "true" means a property was edited
	private:
		void reset_properties(Model::Resource* resource);

		Core::Editor& m_editor;

		UI::Developer::ChildWindow m_child_window;

		std::vector<PropertyEditor::Instance> m_property_editors;
		bool m_read_only;

		Model::Resource* m_resource;
	};

	class ResourceEditorWindow
	{
	private:
		ResourceEditorWindow(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);
		void update_label(Model::Resource* resource);

		Core::Editor& m_editor;

		UI::Developer::Window m_window;

		ResourceEditorWidget m_editor_widget;
		std::string m_label;

		UI::Developer::Button m_save_button; // FIXME: save automatically? Or use some other logic?

		friend class MainWindow;
	};
}
#endif