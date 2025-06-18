#ifndef VADONDEMO_UI_UISYSTEM_HPP
#define VADONDEMO_UI_UISYSTEM_HPP
#include <VadonDemo/UI/MainWindow.hpp>
#include <VadonApp/UI/Developer/GUIElements.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <functional>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonApp::UI::Developer
{
	class GUISystem;
}
namespace VadonDemo::UI
{
	class UI;

	class UISystem
	{
	public:
		using DevGUICallback = std::function<void(VadonApp::UI::Developer::GUISystem&)>;
		using ConsoleCommandCallback = std::function<void(std::string_view)>;

		~UISystem();

		MainWindow& get_main_window() { return m_main_window; }

		void register_dev_gui_callback(const DevGUICallback& callback);
		bool is_dev_gui_enabled() const { return m_dev_gui_enabled; }

		void register_console_command(std::string_view command_name, const ConsoleCommandCallback& callback);
		void show_console();
	private:
		class GameLogger;

		UISystem(Core::GameCore& core);

		bool initialize();
		void update();

		void update_dirty_entities();

		void init_game_ui();
		void init_dev_gui();
		void update_dev_gui();

		void init_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void start_game();
		void return_to_main_menu();
		void load_main_menu();

		Core::GameCore& m_game_core;
		std::unique_ptr<GameLogger> m_logger;

		MainWindow m_main_window;
		std::vector<DevGUICallback> m_dev_gui_callbacks;
		bool m_dev_gui_enabled = false;

		std::unordered_map<std::string, ConsoleCommandCallback> m_console_commands;

		bool m_was_clicked = false;
		bool m_entities_dirty = false;

		Vadon::ECS::EntityHandle m_main_menu_entity;

		Vadon::Utilities::Vector2i m_mouse_pos;
		VadonApp::UI::Developer::Window m_dev_gui_window;

		friend Core::GameCore;
	};
}
#endif