#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/SRV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

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
	private:
		using ShaderPool = Vadon::Utilities::ObjectPool<Vadon::Render::Shader, Shader>;
		using VertexLayoutPool = Vadon::Utilities::ObjectPool<Vadon::Render::VertexLayout, VertexLayout>;

		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		ShaderHandle create_vertex_shader(const ShaderInfo& shader_info);
		ShaderHandle create_pixel_shader(const ShaderInfo& shader_info);

		D3DBlob compile_shader(const ShaderInfo& shader_info);

		GraphicsAPI& m_graphics_api;
		
		ShaderPool m_shader_pool;
		VertexLayoutPool m_layout_pool;

		friend GraphicsAPI;
	};
}
#endif