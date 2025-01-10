#ifndef VADONDEMO_UI_MAINWINDOW_HPP
#define VADONDEMO_UI_MAINWINDOW_HPP
#include <Vadon/Core/Task/Task.hpp>
#include <memory>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::UI
{
	class MainWindow
	{
	public:
		~MainWindow();
	private:
		MainWindow(Core::GameCore& game_core);

		bool initialize();
		void update();
		void render();

		void show_dev_gui();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif