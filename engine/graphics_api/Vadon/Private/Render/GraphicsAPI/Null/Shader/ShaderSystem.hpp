#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Null/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Shader/Shader.hpp>
namespace Vadon::Private::Render::Null
{
	class GraphicsAPI;

	using ShaderResourceSpan = Vadon::Render::ShaderResourceSpan;

	class ShaderSystem : public Vadon::Render::ShaderSystem
	{
	public:
		ShaderHandle create_shader(const ShaderInfo& shader_info) override;
		bool is_shader_valid(ShaderHandle /*shader_handle*/) const override { return false; }
		void apply_shader(ShaderHandle shader_handle) override;
		void remove_shader(ShaderHandle shader_handle) override;

		VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) override;
		bool is_vertex_layout_valid(VertexLayoutHandle /*layout_handle*/) const override { return false; }
		void set_vertex_layout(VertexLayoutHandle layout_handle) override;

		ResourceViewInfo get_resource_view_info(ResourceViewHandle resource_view_handle) const override;
		void apply_resource(ShaderType shader_type, ResourceViewHandle resource_view_handle, int32_t slot) override;
		void apply_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) override;

		bool is_resource_valid(ResourceViewHandle /*resource_view_handle*/) const override { return false; }
		void remove_resource(ResourceViewHandle resource_view_handle) override;
	private:
		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif