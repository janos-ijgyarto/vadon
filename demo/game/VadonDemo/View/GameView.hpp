#ifndef VADONDEMO_VIEW_GAMEVIEW_HPP
#define VADONDEMO_VIEW_GAMEVIEW_HPP
#include <memory>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::View
{
	class GameView
	{
	public:
		~GameView();

		int get_frame_count() const;
	private:
		GameView(Core::GameCore& core);
		
		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif