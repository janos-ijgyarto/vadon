#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#include <QObject>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class SceneTree;
	class SceneManager : public QObject
	{
		Q_OBJECT
	public:
	private slots:
		void asset_opened(const QModelIndex& index);
		void scene_modified(const QUuid& scene_id);
	private:
		SceneManager(Core::Application& application);

		bool initialize();
		void shutdown();

		bool request_close();
		void force_close();

		SceneTree* find_scene_tab(const QUuid& scene_id) const;

		Core::Application& m_application;

		friend class UISystem;
	};
}
#endif