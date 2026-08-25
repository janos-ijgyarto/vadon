#ifndef VADONEDITOR_UI_MODEL_SCENE_ADDCOMPONENTDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ADDCOMPONENTDIALOG_HPP
#include <VadonEditor/UI/Model/Scene/ui_AddComponentDialog.h>
#include <QUuid>
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
		AddComponentDialog(Core::Application& application, const QList<QUuid>& existing_components, QWidget* parent = nullptr);
		void accept() override;
	signals:
		void component_type_selected(const QUuid& component_type_id);
	private slots:
		void filter_text_changed(const QString& text);

		void component_selection_changed();
	private:
		void initialize();
		bool validate_component_selection();

		Ui::AddComponentDialog m_ui;

		Core::Application& m_application;
		QList<QUuid> m_existing_components;
	};
}
#endif