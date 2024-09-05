#ifndef VADONEDITOR_VIEW_MAIN_MAINWINDOW_HPP
#define VADONEDITOR_VIEW_MAIN_MAINWINDOW_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <memory>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class MainWindow
	{
	public:
		~MainWindow();
	private:
		MainWindow(Core::Editor& editor);

		bool initialize();
		void draw(UI::Developer::GUISystem& dev_gui);

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend class ViewSystem;
	};
}
#endif