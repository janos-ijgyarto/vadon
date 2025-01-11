#ifndef VADONDEMO_UI_MAINWINDOW_HPP
#define VADONDEMO_UI_MAINWINDOW_HPP
#include <memory>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::UI
{
	class UISystem;

	class MainWindow
	{
	public:
		~MainWindow();

		void show();
	private:
		MainWindow(Core::GameCore& game_core);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend UISystem;
	};
}
#endif