#ifndef VADONDEMO_UI_UI_HPP
#define VADONDEMO_UI_UI_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Core/File/Path.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
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
	class UI
	{
	public:
		VADONDEMO_API UI(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~UI();

		VADONDEMO_API bool initialize();

		VADONDEMO_API void update(Vadon::ECS::World& ecs_world);

		VADONDEMO_API void update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif