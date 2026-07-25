#ifndef VADONEDITOR_UI_PROJECT_DATASCHEMADIALOG_HPP
#define VADONEDITOR_UI_PROJECT_DATASCHEMADIALOG_HPP
#include <VadonEditor/Utilities/Data/TreeModel.hpp>
#include <QDialog>
#include <QIdentityProxyModel>
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
	private slots:
		void tree_item_selected(const QModelIndex& index);
		void tree_selection_current_changed(const QModelIndex& current, const QModelIndex& previous);
	private:
		void initialize();

		Core::Application& m_application;
		Ui::DataSchemaDialog m_ui;
		QIdentityProxyModel m_type_tree_model;

		Utilities::TreeModel m_schema_model;
	};
}
#endif