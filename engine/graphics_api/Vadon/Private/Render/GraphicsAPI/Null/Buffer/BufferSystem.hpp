#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_BUFFER_BUFFERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
namespace Vadon::Private::Render::Null
{
	class GraphicsAPI;

	class BufferSystem : public Vadon::Render::BufferSystem
	{
	public:
		Vadon::Render::BufferHandle create_buffer(const Vadon::Render::BufferInfo& buffer_info, const void* init_data = nullptr) override;
		bool is_buffer_valid(Vadon::Render::BufferHandle /*buffer_handle*/) const override { return false; }
		void remove_buffer(Vadon::Render::BufferHandle buffer_handle) override;

		Vadon::Render::SRVHandle create_buffer_srv(Vadon::Render::BufferHandle buffer_handle, const Vadon::Render::BufferSRVInfo& buffer_srv_info) override;

		bool buffer_data(Vadon::Render::BufferHandle buffer_handle, const Vadon::Render::BufferWriteData& write_data) override;

		void set_vertex_buffer(Vadon::Render::BufferHandle buffer_handle, int32_t slot, int32_t stride, int32_t offset) override;
		void set_vertex_buffer_slots(const Vadon::Render::VertexBufferSpan& vertex_buffers) override;

		void set_index_buffer(Vadon::Render::BufferHandle buffer_handle, Vadon::Render::GraphicsAPIDataFormat format, int32_t offset) override;

		void set_constant_buffer(Vadon::Render::ShaderType shader, Vadon::Render::BufferHandle buffer_handle, int32_t slot) override;
		void set_constant_buffer_slots(Vadon::Render::ShaderType shader, const Vadon::Render::ConstantBufferSpan& constant_buffers) override;
	protected:
		BufferSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif