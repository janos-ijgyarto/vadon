#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Pipeline/PipelineSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Dummy/GraphicsAPI.hpp>

namespace Vadon::Private::Render::Dummy
{
	Vadon::Render::BlendStateHandle PipelineSystem::get_blend_state(const Vadon::Render::BlendInfo& /*blend_info*/)
	{
		// TODO!!!
		return Vadon::Render::BlendStateHandle();
	}

	void PipelineSystem::apply_blend_state(const Vadon::Render::BlendStateUpdate& /*blend_update*/)
	{
		// TODO!!!
	}

	void PipelineSystem::remove_blend_state(Vadon::Render::BlendStateHandle /*blend_handle*/)
	{
		// TODO!!!
	}

	Vadon::Render::RasterizerStateHandle PipelineSystem::get_rasterizer_state(const Vadon::Render::RasterizerInfo& /*rasterizer_info*/)
	{
		// TODO!!!
		return Vadon::Render::RasterizerStateHandle();
	}

	void PipelineSystem::apply_rasterizer_state(Vadon::Render::RasterizerStateHandle /*rasterizer_handle*/)
	{
		// TODO!!!
	}

	void PipelineSystem::remove_rasterizer_state(Vadon::Render::RasterizerStateHandle /*rasterizer_handle*/)
	{
		// TODO!!!
	}

	Vadon::Render::DepthStencilStateHandle PipelineSystem::get_depth_stencil_state(const Vadon::Render::DepthStencilInfo& /*depth_stencil_info*/)
	{
		// TODO!!!
		return Vadon::Render::DepthStencilStateHandle();
	}

	void PipelineSystem::apply_depth_stencil_state(const Vadon::Render::DepthStencilUpdate& /*depth_stencil_update*/)
	{
		// TODO!!!
	}

	void PipelineSystem::remove_depth_stencil_state(Vadon::Render::DepthStencilStateHandle /*depth_stencil_handle*/)
	{
		// TODO!!!
	}

	void PipelineSystem::set_primitive_topology(Vadon::Render::PrimitiveTopology /*topology*/)
	{
		// TODO!!!
	}

	void PipelineSystem::set_scissor(const Vadon::Utilities::RectangleInt& /*scissor_rectangle*/)
	{
		// TODO!!!
	}

	PipelineSystem::PipelineSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::PipelineSystem(core)
		, m_graphics_api(graphics_api)
	{

	}

	bool PipelineSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void PipelineSystem::shutdown()
	{
		// TODO!!!
	}
}