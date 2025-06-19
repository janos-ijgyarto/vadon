#include <Vadon/Private/PCH/Render.hpp>
#include <Vadon/Private/Render/RenderSystem.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>

namespace Vadon::Private::Render
{
	RenderSystem::RenderSystem(Vadon::Core::EngineCoreInterface& core)
		: m_core(core)
		, m_canvas_system(core)
		, m_frame_system(core)
		, m_text_system(core)
	{

	}

	RenderSystem::~RenderSystem() = default;

	bool RenderSystem::initialize()
	{
		using Logger = Vadon::Core::Logger;
		Logger::log_message("Initializing render system.\n");

		if (m_canvas_system.initialize() == false)
		{
			Logger::log_error("Render system initialization failed.\n");
			return false;
		}

		if (m_frame_system.initialize() == false)
		{
			Logger::log_error("Render system initialization failed.\n");
			return false;
		}

		if (m_text_system.initialize() == false)
		{
			Logger::log_error("Render system initialization failed.\n");
			return false;
		}

		Logger::log_message("Render system initialized successfully.\n");
		return true;
	}

	void RenderSystem::shutdown()
	{
		using Logger = Vadon::Core::Logger;
		Logger::log_message("Shutting down render system.\n");

		m_frame_system.shutdown();

		Logger::log_message("Render system shut down successfully.\n");
	}

	void RenderSystem::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}
}