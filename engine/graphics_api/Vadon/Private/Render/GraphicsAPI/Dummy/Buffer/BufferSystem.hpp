#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_BUFFER_BUFFERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_BUFFER_BUFFERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	class BufferSystem : public Vadon::Render::BufferSystem
	{
	public:
		Vadon::Render::BufferHandle create_buffer(const Vadon::Render::BufferInfo& buffer_info, const void* init_data = nullptr) override;
		void remove_buffer(Vadon::Render::BufferHandle buffer_handle) override;

		bool buffer_data(Vadon::Render::BufferHandle buffer_handle, const Vadon::Utilities::DataRange& range, const void* data, bool force_discard = false) override;

		// FIXME: allow setting slots more efficiently!!!
		void set_vertex_buffer(Vadon::Render::BufferHandle buffer_handle, int slot) override;
		void set_index_buffer(Vadon::Render::BufferHandle buffer_handle, Vadon::Render::GraphicsAPIDataFormat format) override;
		void set_constant_buffer(Vadon::Render::BufferHandle buffer_handle, int slot) override;

		Vadon::Render::ShaderResourceViewHandle create_shader_resource_view(Vadon::Render::BufferHandle buffer_handle, const Vadon::Render::ShaderResourceViewInfo& srv_info) override;
	protected:
		BufferSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif