#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_SHADERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <span>
namespace Vadon::Render
{
	struct ShaderResourceSpan
	{
		int32_t start_slot = 0;
		std::span<ResourceViewHandle> resources;
	};

	class ShaderSystem : public GraphicsSystem<ShaderSystem>
	{
	public:
		virtual ShaderHandle create_shader(const ShaderInfo& shader_info) = 0;
		virtual bool is_shader_valid(ShaderHandle shader_handle) const = 0;
		virtual void apply_shader(ShaderHandle shader_handle) = 0;
		virtual void remove_shader(ShaderHandle shader_handle) = 0;

		virtual VertexLayoutHandle create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info) = 0;
		virtual bool is_vertex_layout_valid(VertexLayoutHandle layout_handle) const = 0;
		virtual void set_vertex_layout(VertexLayoutHandle layout_handle) = 0;

		// TODO: use std::span to set multiple resources in one go!
		virtual ResourceViewInfo get_resource_view_info(ResourceViewHandle resource_view_handle) const = 0;
		
		virtual void apply_resource(ShaderType shader_type, ResourceViewHandle resource_view_handle, int32_t slot) = 0;
		virtual void apply_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) = 0;

		virtual bool is_resource_valid(ResourceViewHandle resource_view_handle) const = 0;
		virtual void remove_resource(ResourceViewHandle resource_view_handle) = 0;
	protected:
		ShaderSystem(Core::EngineCoreInterface& core) 
			: System(core) 
		{
		}
	};
}
#endif