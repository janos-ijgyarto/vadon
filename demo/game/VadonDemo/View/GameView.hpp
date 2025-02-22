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

		bool init_canvas();

		void update_camera();

		void init_entity(Vadon::ECS::EntityHandle entity);

		void init_resource(VadonDemo::View::ViewResourceHandle resource_handle);
		void update_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle);
		void load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle);

		Core::GameCore& m_game_core;

		int m_view_frame_count = 0;

		std::vector<Vadon::ECS::EntityHandle> m_deferred_init_queue;

		friend Core::GameCore;
	};
}
#endif