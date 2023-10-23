#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/Buffer.hpp>

namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class BufferSystem final : public Vadon::Render::BufferSystem
	{
	public:
		BufferHandle create_buffer(const BufferInfo& buffer_info, const void* init_data = nullptr) override;
		void remove_buffer(BufferHandle buffer_handle) override;

		bool buffer_data(BufferHandle buffer_handle, const Vadon::Utilities::DataRange& range, const void* data, bool force_discard = false) override;

		void set_vertex_buffer(BufferHandle buffer_handle, int slot) override;
		void set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format) override;
		void set_constant_buffer(BufferHandle buffer_handle, int slot) override;

		ShaderResourceViewHandle create_shader_resource_view(BufferHandle buffer_handle, const ShaderResourceViewInfo& srv_info) override;
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