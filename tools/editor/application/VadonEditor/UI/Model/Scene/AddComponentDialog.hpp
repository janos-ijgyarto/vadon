#ifndef VADONEDITOR_UI_MODEL_SCENE_ADDCOMPONENTDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ADDCOMPONENTDIALOG_HPP
#include <VadonEditor/UI/Model/Scene/ui_AddComponentDialog.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Entity;
}
namespace VadonEditor::UI
{
	class AddComponentDialog : public QDialog
	{
		Q_OBJECT
	public:
		AddComponentDialog(Core::Application& application, Model::Entity* entity, QWidget* parent);
	signals:
		void component_type_selected(const QUuid& component_type_id);
	private slots:
		void component_selection_changed();
		void finalize_component_selection();
	private:
		void initialize();

		Ui::AddComponentDialog m_ui;

		Core::Application& m_application;
		Model::Entity* m_entity;
	};
}
#endif