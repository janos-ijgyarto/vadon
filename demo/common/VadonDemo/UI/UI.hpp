#ifndef VADONDEMO_UI_UI_HPP
#define VADONDEMO_UI_UI_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Core/File/Path.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
#include <functional>
#include <memory>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::UI
{
	// TODO: more advanced input handling!
	struct CursorState
	{
		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		bool clicked = false;
	};

	class UI
	{
	public:
		using SelectableCallback = std::function<void(std::string_view)>;

		VADONDEMO_API UI(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~UI();

		static void register_types();
		VADONDEMO_API bool initialize();

		VADONDEMO_API void register_selectable_callback(std::string_view key, SelectableCallback callback);

		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, const CursorState& cursor);

		VADONDEMO_API void update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif