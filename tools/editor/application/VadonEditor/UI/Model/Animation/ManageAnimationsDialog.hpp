#ifndef VADONEDITOR_UI_MODEL_ANIMATION_MANAGEANIMATIONSDIALOG_HPP
#define VADONEDITOR_UI_MODEL_ANIMATION_MANAGEANIMATIONSDIALOG_HPP
#include <VadonEditor/UI/Model/Animation/ui_ManageAnimationsDialog.h>
#include <QStyledItemDelegate>
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::UI
{
	class ManageAnimationsDialog : public QDialog
	{
		Q_OBJECT
	public:
		ManageAnimationsDialog(Model::Resource* resource, QWidget* parent = nullptr);
	signals:
		void animation_added(const QUuid& id);
		void animation_edited(const QUuid& id);
		void animation_removed(const QUuid& id);
	private slots:
		void add_animation_clicked();
		void remove_animation_clicked();

		void animation_name_edited(const QString& text);

		void animation_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
	private:
		void update_controls();
		QListWidgetItem* get_selected_item() const;

		Ui::ManageAnimationsDialog m_ui;

		Model::Resource* m_resource;
	};
}
#endif