#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Dummy/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Shader/Shader.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	class ShaderSystem : public Vadon::Render::ShaderSystem
	{
	public:
		ShaderHandle create_shader(const ShaderInfo& shader_info) override;
		void apply_shader(ShaderHandle shader_handle) override;
		void remove_shader(ShaderHandle shader_handle) override;

		VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) override;
		void set_vertex_layout(VertexLayoutHandle layout_handle) override;

		ShaderResourceViewInfo get_resource_view_info(ShaderResourceViewHandle srv_handle) const override;
		void apply_resource(ShaderType shader_type, ShaderResourceViewHandle resource_handle, int32_t slot) override;
		void remove_resource(ShaderResourceViewHandle srv_handle) override;
	private:
		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif