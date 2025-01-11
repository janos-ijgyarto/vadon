#ifndef VADONDEMO_MODEL_GAMEMODEL_HPP
#define VADONDEMO_MODEL_GAMEMODEL_HPP
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
		~GameModel();

		float get_sim_timestep() const;
		float get_accumulator() const;

		int get_frame_count() const;

		// True if the model was updated this frame
		// FIXME: implement as a proper event/signal!
		bool is_updated() const;
	private:
		GameModel(Core::GameCore& core, Vadon::Core::EngineEnvironment& environment);
		
		bool initialize();
		bool init_simulation();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif