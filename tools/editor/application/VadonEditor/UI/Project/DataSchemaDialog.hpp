#ifndef VADONEDITOR_UI_PROJECT_DATASCHEMADIALOG_HPP
#define VADONEDITOR_UI_PROJECT_DATASCHEMADIALOG_HPP
#include <VadonEditor/Utilities/Data/TreeModel.hpp>
#include <QObject>
#include <QDialog>
#include <QGroupBox>
#include <VadonEditor/UI/Project/ui_DataSchemaDialog.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class DataSchemaDialog : public QDialog
	{
		Q_OBJECT
	public:
		DataSchemaDialog(Core::Application& application, QWidget* parent = nullptr);
		~DataSchemaDialog();
	private slots:
		void type_selection_changed();
	private:
		void initialize();

		Core::Application& m_application;
		Ui::DataSchemaDialog m_ui;

		Utilities::TreeModel m_schema_model;
	};
}
#endif