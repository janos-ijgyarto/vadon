#ifndef VADONDEMO_MODEL_GAMEMODEL_HPP
#define VADONDEMO_MODEL_GAMEMODEL_HPP
#include <Vadon/Scene/Scene.hpp>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::Model
{
	class GameModel
	{
	public:
		enum class State
		{
			INIT,
			LOADING,
			RUNNING
		};

		enum class SimState
		{
			INVALID,
			PLAYING,
			PAUSED,
			GAME_OVER
			// TODO: other states?
		};

		struct LevelConfiguration
		{
			Vadon::Scene::SceneID scene_id;
			// TODO: any other parameters?
		};

		~GameModel();

		State get_state() const { return m_state; }
		SimState get_sim_state() const { return m_sim_state; }

		bool load_level(const LevelConfiguration& level_config);
		void set_paused(bool paused);
		void quit_level();

		float get_sim_timestep() const;
		float get_accumulator() const { return m_model_accumulator; }

		int get_frame_count() const { return m_model_frame_count; }

		// FIXME: have a more elegant info query system!
		int get_player_health() const;

		// True if the model was updated this frame
		// FIXME: implement as a proper event/signal!
		bool is_updated() const { return m_model_updated; }
	private:
		GameModel(Core::GameCore& core);
		
		bool initialize();
		void update();

		void update_player_input();

		Core::GameCore& m_game_core;

		bool m_model_updated = false;

		State m_state = State::INIT;
		SimState m_sim_state = SimState::INVALID;

		float m_model_accumulator = 0.0f;
		int m_model_frame_count = 0;

		friend Core::GameCore;
	};
}
#endif