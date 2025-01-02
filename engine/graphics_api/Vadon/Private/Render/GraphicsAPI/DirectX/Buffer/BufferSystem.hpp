#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/Buffer.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/SRV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>

namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	using BufferWriteData = Vadon::Render::BufferWriteData;
	using VertexBufferInfo = Vadon::Render::VertexBufferInfo;
	using VertexBufferSpan = Vadon::Render::VertexBufferSpan;
	using ConstantBufferSpan = Vadon::Render::ConstantBufferSpan;

	class BufferSystem final : public Vadon::Render::BufferSystem
	{
	public:
		BufferHandle create_buffer(const BufferInfo& buffer_info, const void* init_data = nullptr) override;
		bool is_buffer_valid(BufferHandle buffer_handle) const override;
		void remove_buffer(BufferHandle buffer_handle) override;

		SRVHandle create_buffer_srv(BufferHandle buffer_handle, const BufferSRVInfo& buffer_srv_info) override;

		bool buffer_data(BufferHandle buffer_handle, const BufferWriteData& write_data) override;

		void set_vertex_buffer(BufferHandle buffer_handle, int32_t slot, int32_t stride, int32_t offset) override;
		void set_vertex_buffer_slots(const VertexBufferSpan& vertex_buffers) override;

		void set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format, int32_t offset) override;

		void set_constant_buffer(ShaderType shader, BufferHandle buffer_handle, int32_t slot) override;
		void set_constant_buffer_slots(ShaderType shader, const ConstantBufferSpan& constant_buffers) override;
	private:
		using BufferPool = Vadon::Utilities::ObjectPool<Vadon::Render::Buffer, Buffer>;

		BufferSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		BufferPool m_buffer_pool;

		friend GraphicsAPI;
	};
}
#endif