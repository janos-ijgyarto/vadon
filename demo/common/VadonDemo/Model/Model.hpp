#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineCoreInterface;
	class EngineEnvironment;
}
namespace Vadon::ECS
{
	class World;
	struct ComponentEvent;
}
namespace VadonDemo::Model
{
	struct PlayerInput;

	class Model
	{
	public:
		VADONDEMO_API Model(Vadon::Core::EngineCoreInterface& engine_core);
		VADONDEMO_API ~Model();

		VADONDEMO_API bool initialize();

		VADONDEMO_API bool init_simulation(Vadon::ECS::World& ecs_world);
		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, float delta_time);

		VADONDEMO_API static void init_engine_environment(Vadon::Core::EngineEnvironment& environment);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif