#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
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

		void apply_resource(ShaderType shader_type, ShaderResourceHandle resource_handle, int32_t slot) override;

		ShaderResourceHandle create_resource(ID3D11Resource* resource, const ShaderResourceInfo& resource_info);
		void remove_resource(ShaderResourceHandle resource_handle);
	private:
		using ShaderPool = Vadon::Utilities::ObjectPool<Vadon::Render::Shader, Shader>;
		using VertexLayoutPool = Vadon::Utilities::ObjectPool<Vadon::Render::VertexLayout, VertexLayout>;
		using ShaderResourcePool = Vadon::Utilities::ObjectPool<Vadon::Render::ShaderResource, ShaderResource>;

		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		ShaderHandle create_vertex_shader(const ShaderInfo& shader_info);
		ShaderHandle create_pixel_shader(const ShaderInfo& shader_info);

		D3DBlob compile_shader(const ShaderInfo& shader_info);

		GraphicsAPI& m_graphics_api;
		
		ShaderPool m_shader_pool;
		VertexLayoutPool m_layout_pool;
		ShaderResourcePool m_resource_pool;

		std::array<ShaderHandle, Vadon::Utilities::to_integral(ShaderType::TYPE_COUNT)> m_current_shaders;

		friend GraphicsAPI;
	};
}
#endif