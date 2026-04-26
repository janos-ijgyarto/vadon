#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#include <QHash>
#include <QObject>
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
	class SceneManager : public QObject
	{
		Q_OBJECT
	public:
	private slots:
		void asset_opened(const QModelIndex& index);
		void entity_opened(const QUuid& scene_id, const QModelIndex& index);
		void entity_widget_removed(QObject* widget_obj);
	private:
		SceneManager(Core::Application& application);

		bool initialize();

		Core::Application& m_application;

		QHash<Model::Entity*, QWidget*> m_entity_widgets;
		QHash<QWidget*, Model::Entity*> m_widget_reverse_lookup;

		friend class UISystem;
	};
}
#endif