#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Shader/ShaderSystem.hpp>

namespace Vadon::Private::Render::Null
{
	ShaderHandle ShaderSystem::create_shader(const ShaderInfo& /*shader_info*/)
	{
		// TODO!!!
		return ShaderHandle();
	}
	
	void ShaderSystem::apply_shader(ShaderHandle /*shader_handle*/)
	{
		// TODO!!!
	}

	void ShaderSystem::remove_shader(ShaderHandle /*shader_handle*/)
	{
		// TODO!!!
	}

	VertexLayoutHandle ShaderSystem::create_vertex_layout(ShaderHandle /*shader_handle*/, const VertexLayoutInfo& /*layout_info*/)
	{
		// TODO!!!
		return VertexLayoutHandle();
	}

	void ShaderSystem::set_vertex_layout(VertexLayoutHandle /*layout_handle*/)
	{
		// TODO!!!
	}

	ShaderSystem::ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::ShaderSystem(core)
		, m_graphics_api(graphics_api)
	{

	}
}