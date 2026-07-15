#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#include <VadonEditor/UI/Model/Scene/ui_SceneTree.h>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Entity;
	class Scene;
}
namespace VadonEditor::UI
{
	class SceneTree : public QWidget
	{
		Q_OBJECT
	public:
		SceneTree(Model::Scene* scene, QWidget* parent = nullptr);

		QString get_label() const;
		Model::Scene* get_scene() const { return m_scene; }
	signals:
		void scene_modified(const QUuid& scene_id);
		void scene_saved(const QUuid& scene_id);
	private slots:
		void entity_double_clicked(const QModelIndex& index);
		void entity_widget_removed(QObject* widget_obj);

		void save_triggered();

		void add_entity_triggered();
		void remove_entity_triggered();

		void entity_context_menu_requested(const QPoint& position);

		void internal_scene_modified();
	private:
		void entity_opened(Model::Entity* entity);

		void internal_close();

		void set_modified();

		Model::Entity* get_selected_entity() const;

		Ui::SceneTree m_ui;

		Model::Scene* m_scene;

		QHash<QUuid, QWidget*> m_entity_widgets;
		QHash<QWidget*, QUuid> m_widget_reverse_lookup;

		friend class SceneManager;
	};
}
#endif