#ifndef VADONEDITOR_UI_MODEL_SCENE_SELECTCOMPONENTPROPERTYDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SELECTCOMPONENTPROPERTYDIALOG_HPP
#include <VadonEditor/UI/Model/Scene/ui_SelectComponentPropertyDialog.h>
#include <QStandardItemModel>
#include <QUuid>
namespace VadonEditor::Model
{
	class Entity;
}
namespace VadonEditor::UI
{
	class SelectComponentPropertyDialog : public QDialog
	{
		Q_OBJECT
	public:
		SelectComponentPropertyDialog(const Model::Entity* owner_entity, QWidget* parent = nullptr);
	signals:
		void component_property_selected(const QUuid& entity, const QUuid& component, const QUuid& property);
	private slots:
		void ok_clicked();
		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void tree_item_double_clicked(const QModelIndex& index);
	private:
		void update_controls();
		QStandardItem* get_selected_item() const;

		void finalize_selection(const QStandardItem* item);

		Ui::SelectComponentPropertyDialog m_ui;

		const Model::Entity* m_owner_entity;
		QStandardItemModel m_component_property_model;
	};
}
#endif