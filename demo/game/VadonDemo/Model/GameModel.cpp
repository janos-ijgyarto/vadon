#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/UI/UISystem.hpp>

#include <VadonApp/Core/Application.hpp>
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
	struct GameModel::Internal
	{
		Core::GameCore& m_game_core;

		std::unique_ptr<Model> m_model;
		bool m_model_updated = false;

		State m_state = State::INIT;
		SimState m_sim_state = SimState::INVALID;

		float m_model_accumulator = 0.0f;
		int m_model_frame_count = 0;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{
		}

		bool initialize()
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			m_model = std::make_unique<Model>(engine_core);

			if (m_model->initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool load_level(const LevelConfiguration& level_config)
		{
			m_state = State::LOADING;

			Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
			if (m_model->init_simulation(ecs_world, level_config.scene_id) == false)
			{
				// TODO: error?
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

		void set_paused(bool paused)
		{
			if (m_state == State::RUNNING)
			{
				m_sim_state = paused ? SimState::PAUSED : SimState::PLAYING;
			}
		}

		void quit_level()
		{
			if (m_state == State::INIT)
			{
				return;
			}

			m_model->end_simulation(m_game_core.get_ecs_world());
			m_state = State::INIT;
			m_sim_state = SimState::INVALID;
		}

		void update()
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

			while (model_accumulator >= c_sim_timestep)
			{
				m_model_updated = true;

				update_player_input();

				Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
				m_model->update(ecs_world, c_sim_timestep);
				if (m_model->is_in_end_state(ecs_world) == true)
				{
					m_sim_state = SimState::GAME_OVER;
				}

				model_accumulator -= c_sim_timestep;				
				++m_model_frame_count;
			}

			m_model_accumulator = model_accumulator;
		}

		void update_player_input()
		{
			// Update movement
			VadonDemo::Model::PlayerInput player_input;

			const Platform::PlatformInterface::InputValues input_values = m_game_core.get_platform_interface().get_input_values();

			UI::UISystem& ui_system = m_game_core.get_ui_system();
			VadonApp::UI::Developer::GUISystem& dev_gui = m_game_core.get_engine_app().get_system<VadonApp::UI::Developer::GUISystem>();
			if ((ui_system.is_dev_gui_enabled() == false) || (Vadon::Utilities::to_bool(dev_gui.get_io_flags() & VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE) == false))
			{
				if (input_values.move_left == true)
				{
					player_input.move_dir.x = -1.0f;
				}
				else if (input_values.move_right == true)
				{
					player_input.move_dir.x = 1.0f;
				}

				if (input_values.move_up == true)
				{
					player_input.move_dir.y = 1.0f;
				}
				else if (input_values.move_down == true)
				{
					player_input.move_dir.y = -1.0f;
				}

				player_input.fire = input_values.fire;
			}

			auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&>();
			auto player_it = player_query.get_iterator();
			if (player_it.is_valid() == true)
			{
				auto player_components = player_it.get_tuple();

				VadonDemo::Model::Player& player_component = std::get<VadonDemo::Model::Player&>(player_components);
				player_component.input = player_input;
			}
		}

		int get_player_health() const
		{
			auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&, VadonDemo::Model::Health&>();
			auto player_it = player_query.get_iterator();
			if (player_it.is_valid() == true)
			{
				auto player_components = player_it.get_tuple();

				VadonDemo::Model::Health& player_health = std::get<VadonDemo::Model::Health&>(player_components);
				return static_cast<int>(player_health.current_health);
			}

			// TODO: error?
			return -1;
		}
	};

	GameModel::~GameModel() = default;

	GameModel::State GameModel::get_state() const
	{
		return m_internal->m_state;
	}

	GameModel::SimState GameModel::get_sim_state() const
	{
		return m_internal->m_sim_state;
	}

	bool GameModel::load_level(const LevelConfiguration& level_config)
	{
		return m_internal->load_level(level_config);
	}

	void GameModel::set_paused(bool paused)
	{
		m_internal->set_paused(paused);
	}

	void GameModel::quit_level()
	{
		m_internal->quit_level();
	}

	float GameModel::get_sim_timestep() const
	{
		return c_sim_timestep;
	}

	float GameModel::get_accumulator() const
	{
		return m_internal->m_model_accumulator;
	}

	int GameModel::get_frame_count() const
	{
		return m_internal->m_model_frame_count;
	}

	int GameModel::get_player_health() const
	{
		return m_internal->get_player_health();
	}

	bool GameModel::is_updated() const
	{
		return m_internal->m_model_updated;
	}

	GameModel::GameModel(Core::GameCore& core)
		: m_internal(std::make_unique<Internal>(core))
	{
	}

	bool GameModel::initialize()
	{
		return m_internal->initialize();
	}

	void GameModel::update()
	{
		m_internal->update();
	}
}