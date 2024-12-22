#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/GraphicsAPI.hpp>

namespace Vadon::Private::Render::Null
{
	GraphicsAPI::GraphicsAPI(Vadon::Core::EngineCoreInterface& core)
		: GraphicsAPIBase(core)
		, m_buffer_system(core, *this)
		, m_pipeline_system(core, *this)
		, m_rt_system(core, *this)
		, m_shader_system(core, *this)
		, m_texture_system(core, *this)
	{

	}

	void GraphicsAPI::draw(const Vadon::Render::DrawCommand& /*command*/)
	{
		// TODO!!!
	}

	bool GraphicsAPI::initialize()
	{
		log_message("Initializing null graphics API.\n");

		// TODO!!!

		log_message("Null graphics API initialized successfully.\n");
		return true;
	}

	void GraphicsAPI::update()
	{
		// TODO!!!
	}

	void GraphicsAPI::shutdown()
	{
		log_message("Shutting down null graphics API.\n");

		// TODO!!!

		log_message("Null graphics API successfully shut down.\n");
	}
}