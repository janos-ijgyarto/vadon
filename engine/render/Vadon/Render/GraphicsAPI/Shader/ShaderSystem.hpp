#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
namespace Vadon::Render
{
	class ShaderSystem : public GraphicsSystem<ShaderSystem>
	{
	public:
		// NOTE: this expects data for a compiled shader that is compatible with the backend
		virtual ShaderHandle create_shader(const ShaderInfo& shader_info, const void* shader_data, size_t shader_data_size) = 0;

		// TODO: for builds that support it, allow the system to compile shaders from source?

		virtual bool is_shader_valid(ShaderHandle shader_handle) const = 0;
		virtual void apply_shader(ShaderHandle shader_handle) = 0;
		virtual void remove_shader(ShaderHandle shader_handle) = 0;

		// NOTE: layout needs to be created with a "reference" shader, but can then be used with other compatible shaders
		virtual VertexLayoutHandle create_vertex_layout(const VertexLayoutInfo& layout_info, const void* shader_data, size_t shader_data_size) = 0;
		virtual bool is_vertex_layout_valid(VertexLayoutHandle layout_handle) const = 0;
		virtual void set_vertex_layout(VertexLayoutHandle layout_handle) = 0;

		// TODO: shader reflection (resource slots, etc.)
	protected:
		ShaderSystem(Core::EngineCoreInterface& core) 
			: System(core) 
		{
		}
	};
}
#endif