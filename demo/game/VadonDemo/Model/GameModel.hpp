#ifndef VADONDEMO_MODEL_GAMEMODEL_HPP
#define VADONDEMO_MODEL_GAMEMODEL_HPP
#include <Vadon/Scene/Scene.hpp>
#include <memory>
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

		State get_state() const;
		SimState get_sim_state() const;

		bool load_level(const LevelConfiguration& level_config);
		void set_paused(bool paused);
		void quit_level();

		float get_sim_timestep() const;
		float get_accumulator() const;

		int get_frame_count() const;

		// FIXME: have a more elegant info query system!
		int get_player_health() const;

		// True if the model was updated this frame
		// FIXME: implement as a proper event/signal!
		bool is_updated() const;
	private:
		GameModel(Core::GameCore& core);
		
		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif