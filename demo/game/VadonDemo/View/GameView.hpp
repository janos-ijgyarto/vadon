#ifndef VADONDEMO_VIEW_GAMEVIEW_HPP
#define VADONDEMO_VIEW_GAMEVIEW_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
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

		int get_frame_count() const { return m_view_frame_count; }
	private:
		GameView(Core::GameCore& core);
		
		bool initialize();
		void update();

		void update_dirty_entities();
		void update_camera();

		void init_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void init_resource(ViewResourceID resource_id);

		Core::GameCore& m_game_core;

		int m_view_frame_count = 0;

		friend Core::GameCore;
	};
}
#endif