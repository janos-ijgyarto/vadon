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

	bool BufferSystem::buffer_data(Vadon::Render::BufferHandle /*buffer_handle*/, const Vadon::Render::BufferWriteData& /*write_data*/)
	{
		// TODO!!!
		return false;
	}

	void BufferSystem::set_vertex_buffer(Vadon::Render::BufferHandle /*buffer_handle*/, int32_t /*slot*/, int32_t /*stride*/, int32_t /*offset*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_vertex_buffer_slots(const Vadon::Render::VertexBufferSpan& /*vertex_buffers*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_index_buffer(Vadon::Render::BufferHandle /*buffer_handle*/, Vadon::Render::GraphicsAPIDataFormat /*format*/, int32_t /*offset*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_constant_buffer(Vadon::Render::ShaderType /*shader*/, Vadon::Render::BufferHandle /*buffer_handle*/, int32_t /*slot*/)
	{
		// TODO!!!
	}

	void BufferSystem::set_constant_buffer_slots(Vadon::Render::ShaderType /*shader*/, const Vadon::Render::ConstantBufferSpan& /*constant_buffers*/)
	{
		// TODO!!!
	}

	Vadon::Render::ResourceViewHandle BufferSystem::create_resource_view(Vadon::Render::BufferHandle /*buffer_handle*/, const Vadon::Render::BufferResourceViewInfo& /*resource_view_info*/)
	{
		// TODO!!!
		return Vadon::Render::ResourceViewHandle();
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