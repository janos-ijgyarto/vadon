#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/UI/UISystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

namespace
{
	constexpr float c_sim_timestep = 1.0f / 30.0f;
	constexpr float c_frame_limit = 0.25f;
}

namespace VadonDemo::Model
{
	GameModel::~GameModel() = default;

	bool GameModel::load_level(const LevelConfiguration& level_config)
	{
		m_state = State::LOADING;

		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		VadonDemo::Model::Model& model = m_game_core.get_core().get_model();
		if (model.init_simulation(ecs_world, level_config.scene_id) == false)
		{
			Vadon::Core::Logger::log_error("Failed to load level!");
			m_state = State::INIT;
			m_sim_state = SimState::INVALID;
			return false;
		}

		m_state = State::RUNNING;
		m_sim_state = SimState::PLAYING;
		{
			// TODO: log that we opened a level!
		}

		return true;
	}

	void GameModel::set_paused(bool paused)
	{
		if (m_state == State::RUNNING)
		{
			m_sim_state = paused ? SimState::PAUSED : SimState::PLAYING;
		}
	}

	void GameModel::quit_level()
	{
		if (m_state == State::INIT)
		{
			return;
		}

		VadonDemo::Model::Model& model = m_game_core.get_core().get_model();
		model.end_simulation(m_game_core.get_ecs_world());
		m_state = State::INIT;
		m_sim_state = SimState::INVALID;
	}

	float GameModel::get_sim_timestep() const
	{
		return c_sim_timestep;
	}

	int GameModel::get_player_health() const
	{
		auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&, VadonDemo::Model::Health&>();
		auto player_it = player_query.get_iterator();
		if (player_it.is_valid() == true)
		{
			auto player_health = player_it.get_component<VadonDemo::Model::Health>();
			return static_cast<int>(player_health->current_health);
		}

		// TODO: error?
		return -1;
	}

	GameModel::GameModel(Core::GameCore& core)
		: m_game_core(core)
	{
	}

	bool GameModel::initialize()
	{
		m_game_core.get_core().add_entity_event_callback(
			[this](Vadon::ECS::World& ecs_world, const VadonDemo::Core::EntityEvent& event)
			{
				switch (event.type)
				{
				case VadonDemo::Core::EntityEventType::ADDED:
					m_game_core.get_core().get_model().init_entity_collision(ecs_world, event.entity);
					break;
				}
			}
		);

		return true;
	}

	void GameModel::update()
	{
		// Reset update flag
		m_model_updated = false;

		if (m_state != State::RUNNING)
		{
			return;
		}

		if (m_sim_state != SimState::PLAYING)
		{
			return;
		}

		// Model update logic inspired by the "Fix Your Timestep!" blog by Gaffer on Games: https://gafferongames.com/post/fix_your_timestep/
		const float delta_time = std::min(m_game_core.get_delta_time(), c_frame_limit);

		float model_accumulator = m_model_accumulator;
		model_accumulator += delta_time;

		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();

		while (model_accumulator >= c_sim_timestep)
		{
			m_model_updated = true;

			update_player_input();

			VadonDemo::Model::Model& model = m_game_core.get_core().get_model();
			model.update(ecs_world, c_sim_timestep);
			if (model.is_in_end_state(ecs_world) == true)
			{
				m_sim_state = SimState::GAME_OVER;
			}

			model_accumulator -= c_sim_timestep;
			++m_model_frame_count;
		}

		m_model_accumulator = model_accumulator;
	}

	void GameModel::update_player_input()
	{
		// Update movement
		VadonDemo::Model::PlayerInput player_input;

		Platform::PlatformInterface& game_platform_interface = m_game_core.get_platform_interface();
		VadonApp::Platform::InputSystem& input_system = m_game_core.get_engine_app().get_system<VadonApp::Platform::InputSystem>();

		UI::UISystem& ui_system = m_game_core.get_ui_system();
		VadonApp::UI::Developer::GUISystem& dev_gui = m_game_core.get_engine_app().get_system<VadonApp::UI::Developer::GUISystem>();
		if ((ui_system.is_dev_gui_enabled() == false) || (Vadon::Utilities::to_bool(dev_gui.get_io_flags() & VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE) == false))
		{
			if (input_system.is_action_pressed(game_platform_interface.get_action(Platform::GameInputAction::MOVE_LEFT)) == true)
			{
				player_input.move_dir.x = -1.0f;
			}
			else if (input_system.is_action_pressed(game_platform_interface.get_action(Platform::GameInputAction::MOVE_RIGHT)) == true)
			{
				player_input.move_dir.x = 1.0f;
			}

			if (input_system.is_action_pressed(game_platform_interface.get_action(Platform::GameInputAction::MOVE_UP)) == true)
			{
				player_input.move_dir.y = 1.0f;
			}
			else if (input_system.is_action_pressed(game_platform_interface.get_action(Platform::GameInputAction::MOVE_DOWN)) == true)
			{
				player_input.move_dir.y = -1.0f;
			}
		}

		auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&>();
		auto player_it = player_query.get_iterator();
		if (player_it.is_valid() == true)
		{
			auto player_component = player_it.get_component<VadonDemo::Model::Player>();
			player_component->input = player_input;
		}
	}
}