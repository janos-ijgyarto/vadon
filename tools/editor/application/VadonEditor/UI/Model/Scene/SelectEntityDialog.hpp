#ifndef VADONEDITOR_UI_MODEL_SCENE_SELECTENTITYDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SELECTENTITYDIALOG_HPP
#include <VadonEditor/UI/Model/Scene/ui_SelectEntityDialog.h>
#include <QUuid>
namespace VadonEditor::Model
{
	class Entity;
	class Scene;
}
namespace VadonEditor::UI
{
	class SelectEntityDialog : public QDialog
	{
		Q_OBJECT
	public:
		SelectEntityDialog(Model::Scene* owner_scene, QWidget* parent = nullptr);
	signals:
		void entity_selected(const QUuid& entity);
	private slots:
		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void entity_double_clicked(const QModelIndex& index);
		void ok_clicked();
	private:
		void update_controls();

		Model::Entity* get_selected_entity() const;
		Model::Entity* get_entity(const QModelIndex& index) const;

		void finalize_entity_selection(const Model::Entity* entity);

		Ui::SelectEntityDialog m_ui;

		const Model::Scene* m_owner_scene;
	};
}
#endif