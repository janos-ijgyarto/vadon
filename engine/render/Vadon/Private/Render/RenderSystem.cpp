#include <Vadon/Private/PCH/Render.hpp>
#include <Vadon/Private/Render/RenderSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>

namespace Vadon::Private::Render
{
	RenderSystem::RenderSystem(Vadon::Core::EngineCoreInterface& core)
		: m_core(core)
		, m_canvas_system(core)
		, m_frame_system(core)
	{

	}

	RenderSystem::~RenderSystem() = default;

	bool RenderSystem::initialize()
	{
		Vadon::Core::Logger& logger = m_core.get_logger();
		logger.log("Initializing render system.\n");

		if (m_canvas_system.initialize() == false)
		{
			logger.log("Render system initialization failed.\n");
			return false;
		}

		if (m_frame_system.initialize() == false)
		{
			logger.log("Render system initialization failed.\n");
			return false;
		}

		logger.log("Render system initialized successfully.\n");
		return true;
	}

	void RenderSystem::update()
	{
		m_frame_system.update();
	}

	void RenderSystem::shutdown()
	{
		Vadon::Core::Logger& logger = m_core.get_logger();
		logger.log("Shutting down render system.\n");

		m_frame_system.shutdown();

		logger.log("Render system shut down successfully.\n");
	}

	void RenderSystem::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}
}