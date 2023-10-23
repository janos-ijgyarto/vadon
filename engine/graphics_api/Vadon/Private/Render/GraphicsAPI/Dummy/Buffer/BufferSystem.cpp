#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Buffer/BufferSystem.hpp>
namespace Vadon::Private::Render::Dummy
{
	Vadon::Render::BufferHandle BufferSystem::create_buffer(const Vadon::Render::BufferInfo& /*buffer_info*/, const void* /*init_data*/)
	{
		// TODO!!!
		return Vadon::Render::BufferHandle();
	}

	void BufferSystem::remove_buffer(Vadon::Render::BufferHandle /*buffer_handle*/)
	{
		// TODO!!!
	}

	bool BufferSystem::buffer_data(Vadon::Render::BufferHandle /*buffer_handle*/, const Vadon::Utilities::DataRange& /*range*/, const void* /*data*/, bool /*force_discard*/)
	{
		// TODO!!!
		return true;
	}

	void BufferSystem::set_vertex_buffer(Vadon::Render::BufferHandle /*buffer_handle*/, int /*slot*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_index_buffer(Vadon::Render::BufferHandle /*buffer_handle*/, Vadon::Render::GraphicsAPIDataFormat /*format*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_constant_buffer(Vadon::Render::BufferHandle /*buffer_handle*/, int /*slot*/)
	{
		// TODO!!!
	}

	Vadon::Render::ShaderResourceViewHandle BufferSystem::create_shader_resource_view(Vadon::Render::BufferHandle /*buffer_handle*/, const Vadon::Render::ShaderResourceViewInfo& /*srv_info*/)
	{
		return Vadon::Render::ShaderResourceViewHandle();
	}

	BufferSystem::BufferSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::BufferSystem(core)
		, m_graphics_api(graphics_api)
	{
	}

	bool BufferSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void BufferSystem::shutdown()
	{
		// TODO!!!
	}
}