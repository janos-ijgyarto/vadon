#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFERSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsModule.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
namespace Vadon::Utilities
{
	struct DataRange;
}
namespace Vadon::Render
{
	class BufferSystem : public GraphicsSystem<BufferSystem>
	{
	public:
		virtual BufferHandle create_buffer(const BufferInfo& buffer_info, const void* init_data = nullptr) = 0;
		virtual void remove_buffer(BufferHandle buffer_handle) = 0;

		virtual bool buffer_data(BufferHandle buffer_handle, const Utilities::DataRange& range, const void* data, bool force_discard = false) = 0;

		// FIXME: allow setting slots more efficiently!!!
		virtual void set_vertex_buffer(BufferHandle buffer_handle, int slot) = 0;
		virtual void set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format) = 0;
		virtual void set_constant_buffer(BufferHandle buffer_handle, int slot) = 0;

		virtual ShaderResourceViewHandle create_shader_resource_view(BufferHandle buffer_handle, const ShaderResourceViewInfo& srv_info) = 0;
	protected:
		BufferSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif