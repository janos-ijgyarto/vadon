#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	using ShaderResourceSpan = Vadon::Render::ShaderResourceSpan;

	class ShaderSystem : public Vadon::Render::ShaderSystem
	{
	public:
		ShaderHandle create_shader(const ShaderInfo& shader_info) override;
		bool is_shader_valid(ShaderHandle shader_handle) const override { return m_shader_pool.is_handle_valid(shader_handle); }
		void apply_shader(ShaderHandle shader_handle) override;
		void remove_shader(ShaderHandle shader_handle) override;

		VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) override;
		bool is_vertex_layout_valid(VertexLayoutHandle layout_handle) const override { return m_layout_pool.is_handle_valid(layout_handle); }
		void set_vertex_layout(VertexLayoutHandle layout_handle) override;

		ResourceViewInfo get_resource_view_info(ResourceViewHandle resource_view_handle) const override;
		void apply_resource(ShaderType shader_type, ResourceViewHandle resource_view_handle, int32_t slot) override;
		void apply_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) override;

		bool is_resource_valid(ResourceViewHandle resource_view_handle) const override { return m_resource_pool.is_handle_valid(resource_view_handle); }
		void remove_resource(ResourceViewHandle resource_view_handle) override;

		ResourceViewHandle add_resource_view(D3DShaderResourceView d3d_srv);
		ResourceViewHandle create_resource_view(D3DResourcePtr resource, const ResourceViewInfo& resource_view_info);

		static ResourceViewInfo get_srv_info(D3D11_SHADER_RESOURCE_VIEW_DESC& srv_desc);
	private:
		using ShaderPool = Vadon::Utilities::ObjectPool<Vadon::Render::Shader, Shader>;
		using VertexLayoutPool = Vadon::Utilities::ObjectPool<Vadon::Render::VertexLayout, VertexLayout>;
		using ShaderResourcePool = Vadon::Utilities::ObjectPool<Vadon::Render::ResourceView, ShaderResourceView>;

		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		ShaderHandle create_vertex_shader(const ShaderInfo& shader_info);
		ShaderHandle create_pixel_shader(const ShaderInfo& shader_info);

		D3DBlob compile_shader(const ShaderInfo& shader_info);

		ResourceViewHandle add_resource_view_internal(D3DShaderResourceView d3d_srv, const ResourceViewInfo& resource_view_info);

		GraphicsAPI& m_graphics_api;
		
		ShaderPool m_shader_pool;
		VertexLayoutPool m_layout_pool;
		ShaderResourcePool m_resource_pool;

		std::array<ShaderHandle, Vadon::Utilities::to_integral(ShaderType::TYPE_COUNT)> m_current_shaders;

		friend GraphicsAPI;
	};
}
#endif