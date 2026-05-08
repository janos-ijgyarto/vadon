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
	private slots:
		void entity_double_clicked(const QModelIndex& index);
		void entity_widget_removed(QObject* widget_obj);

		void entity_name_changed(const QUuid& entity_id, const QString& text);

		void entity_component_added(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_removed(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_data_changed(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);

		void save_triggered();
	private:
		void entity_opened(Model::Entity* entity);

		bool request_close();
		void force_close();

		void set_modified();

		Ui::SceneTree m_ui;

		Model::Scene* m_scene;

		QHash<QUuid, QWidget*> m_entity_widgets;
		QHash<QWidget*, QUuid> m_widget_reverse_lookup;

		friend class SceneManager;
	};
}
#endif