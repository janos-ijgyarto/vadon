#ifndef VADONEDITOR_VIEW_SCENE_SCENETREE_HPP
#define VADONEDITOR_VIEW_SCENE_SCENETREE_HPP
#include <memory>
namespace VadonApp::UI::Developer
{
	class GUISystem;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class SceneTreeWindow
	{
	public:
		~SceneTreeWindow();
	private:
		SceneTreeWindow(Core::Editor& editor);

		void on_new_scene_action();
		void on_load_scene_action();
		void on_save_scene_action();

		void draw(VadonApp::UI::Developer::GUISystem& dev_gui);

		Core::Editor& m_editor;

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend class MainWindow;
	};
}
#endif