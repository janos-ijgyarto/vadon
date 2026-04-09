#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENETREE_HPP
#include <VadonEditor/UI/Model/Scene/ui_SceneTree.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class SceneTree : public QWidget
	{
		Q_OBJECT
	public:
		SceneTree(QWidget* parent = nullptr);
	private:
		bool initialize(Core::Application& application);

		Ui::SceneTree m_ui;

		Core::Application* m_application;

		friend class UISystem;
	};
}
#endif