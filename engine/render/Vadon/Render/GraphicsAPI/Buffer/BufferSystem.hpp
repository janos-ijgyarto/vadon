#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFERSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsModule.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <span>
namespace Vadon::Utilities
{
	struct DataRange;
}
namespace Vadon::Render
{
	struct BufferWriteData
	{
		Utilities::DataRange range;
		const void* data = nullptr;
		bool no_overwrite = false;
	};

	struct VertexBufferInfo
	{
		int32_t stride = -1;
		int32_t offset = 0;
		BufferHandle buffer;
	};

	struct VertexBufferSpan
	{
		int32_t start_slot = 0;
		std::span<VertexBufferInfo> buffers;
	};

	struct ConstantBufferSpan
	{
		int32_t start_slot = 0;
		std::span<BufferHandle> buffers;
	};

	class BufferSystem : public GraphicsSystem<BufferSystem>
	{
	public:
		virtual BufferHandle create_buffer(const BufferInfo& buffer_info, const void* init_data = nullptr) = 0;
		virtual bool is_buffer_valid(BufferHandle buffer_handle) const = 0;
		virtual void remove_buffer(BufferHandle buffer_handle) = 0;

		// TODO: extend API to enable mapping, read/write, etc.?
		virtual bool buffer_data(BufferHandle buffer_handle, const BufferWriteData& write_data) = 0;

		virtual void set_vertex_buffer(BufferHandle buffer_handle, int32_t slot, int32_t stride = -1, int32_t offset = 0) = 0;
		virtual void set_vertex_buffer_slots(const VertexBufferSpan& vertex_buffers) = 0;

		virtual void set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format, int32_t offset = 0) = 0;

		virtual void set_constant_buffer(ShaderType shader, BufferHandle buffer_handle, int32_t slot) = 0;
		virtual void set_constant_buffer_slots(ShaderType shader, const ConstantBufferSpan& constant_buffers) = 0;

		virtual ResourceViewHandle create_resource_view(BufferHandle buffer_handle, const BufferResourceViewInfo& resource_view_info) = 0;
	protected:
		BufferSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif