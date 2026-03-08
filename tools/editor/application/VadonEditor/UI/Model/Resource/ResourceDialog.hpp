#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#include <VadonEditor/Core/Project/TypeFilterModel.hpp>
#include <VadonEditor/UI/Model/Resource/ui_NewResourceDialog.h>
#include <QDialog>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class NewResourceDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewResourceDialog(Core::Application& application, QWidget* parent);

		QUuid get_selected_resource_type() const { return m_selected_type; }
	private slots:
		void type_selected(const QModelIndex& index);
		void type_double_clicked(const QModelIndex& index);
	private:
		void validate_state();
		void internal_type_selected(const QModelIndex& index);

		Ui::NewResourceDialog m_ui;

		Core::TypeFilterModel m_type_filter_model;
		QUuid m_selected_type;
	};
}
#endif