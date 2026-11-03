#ifndef VADONEDITOR_UI_MODEL_OBJECT_OBJECTDIALOG_HPP
#define VADONEDITOR_UI_MODEL_OBJECT_OBJECTDIALOG_HPP
#include <VadonEditor/UI/Model/Object/ui_NewObjectDialog.h>
#include <QDialog>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
	class TypeFilterModel;
}
namespace VadonEditor::UI
{
	class NewObjectDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewObjectDialog(Core::Application& application, const QUuid& base_type, QWidget* parent = nullptr);
	signals:
		void object_type_selected(const QUuid& type_uuid);
	private slots:
		void type_double_clicked(const QModelIndex& index);
		void filter_text_changed(const QString& text);

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void selection_accepted();
	private:
		void update_controls();

		QModelIndex get_current_selection() const;
		QUuid get_selected_type(const QModelIndex& index) const;

		void finalize_selection(const QUuid& type_uuid);

		Ui::NewObjectDialog m_ui;

		Core::TypeFilterModel* m_filter_model;
	};
}
#endif