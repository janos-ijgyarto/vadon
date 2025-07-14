#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_SHADER_SHADERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Null/Shader/Shader.hpp>
namespace Vadon::Private::Render::Null
{
	class GraphicsAPI;

	class ShaderSystem : public Vadon::Render::ShaderSystem
	{
	public:
		ShaderHandle create_shader(const ShaderInfo& shader_info, const void* shader_data, size_t shader_data_size) override;
		bool is_shader_valid(ShaderHandle /*shader_handle*/) const override { return false; }
		void apply_shader(ShaderHandle shader_handle) override;
		void remove_shader(ShaderHandle shader_handle) override;

		VertexLayoutHandle create_vertex_layout(const VertexLayoutInfo& layout_info, const void* shader_data, size_t shader_data_size) override;
		bool is_vertex_layout_valid(VertexLayoutHandle /*layout_handle*/) const override { return false; }
		void set_vertex_layout(VertexLayoutHandle layout_handle) override;
	private:
		ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif