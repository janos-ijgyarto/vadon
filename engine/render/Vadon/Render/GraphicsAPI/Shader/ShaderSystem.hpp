#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
namespace Vadon::Render
{
	class ShaderSystem : public GraphicsSystem<ShaderSystem>
	{
	public:
		virtual ShaderHandle create_shader(const ShaderInfo& shader_info) = 0;
		virtual void apply_shader(ShaderHandle shader_handle) = 0;
		virtual void remove_shader(ShaderHandle shader_handle) = 0;

		virtual VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) = 0;
		virtual void set_vertex_layout(VertexLayoutHandle layout_handle) = 0;

		virtual ShaderResourceViewInfo get_resource_view_info(ShaderResourceViewHandle srv_handle) const = 0;
		virtual void apply_resource(ShaderType shader_type, ShaderResourceViewHandle srv_handle, int32_t slot) = 0;
		virtual void remove_resource(ShaderResourceViewHandle srv_handle) = 0;
	protected:
		ShaderSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif