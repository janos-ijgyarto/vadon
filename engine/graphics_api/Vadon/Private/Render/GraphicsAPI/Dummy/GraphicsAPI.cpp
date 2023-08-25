#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/GraphicsAPI.hpp>

namespace Vadon::Private::Render::Dummy
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
		log("Initializing dummy graphics API.\n");

		// TODO!!!

		log("Dummy graphics API initialized successfully.\n");
		return true;
	}

	void GraphicsAPI::update()
	{
		// TODO!!!
	}

	void GraphicsAPI::shutdown()
	{
		log("Shutting down dummy graphics API.\n");

		// TODO!!!

		log("Dummy graphics API successfully shut down.\n");
	}
}