#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Dummy/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Shader/Shader.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	using ShaderResourceSpan = Vadon::Render::ShaderResourceSpan;

	class ShaderSystem : public Vadon::Render::ShaderSystem
	{
	public:
		ShaderHandle create_shader(const ShaderInfo& shader_info) override;
		void apply_shader(ShaderHandle shader_handle) override;
		void remove_shader(ShaderHandle shader_handle) override;

		VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) override;
		void set_vertex_layout(VertexLayoutHandle layout_handle) override;

		ResourceViewInfo get_resource_view_info(ResourceViewHandle resource_view_handle) const override;
		void apply_resource(ShaderType shader_type, ResourceViewHandle resource_view_handle, int32_t slot) override;
		void apply_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) override;

		void remove_resource(ResourceViewHandle resource_view_handle) override;
	private:
		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif