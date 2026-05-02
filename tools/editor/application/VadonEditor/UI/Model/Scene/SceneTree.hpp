#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#include <VadonEditor/UI/Model/Scene/ui_SceneTree.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Scene;
}
namespace VadonEditor::UI
{
	class SceneTree : public QWidget
	{
		Q_OBJECT
	public:
		SceneTree(Model::Scene* scene, QWidget* parent = nullptr);
	signals:
		void entity_opened(const QUuid& scene_id, const QModelIndex& index);
	private slots:
		void entity_double_clicked(const QModelIndex& index);
	private:
		bool close_requested();

		Ui::SceneTree m_ui;

		Model::Scene* m_scene;

		friend class SceneManager;
	};
}
#endif