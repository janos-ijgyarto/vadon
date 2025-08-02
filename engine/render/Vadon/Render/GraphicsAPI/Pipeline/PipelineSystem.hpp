#ifndef VADON_RENDER_GRAPHICSAPI_PIPELINE_PIPELINESYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_PIPELINE_PIPELINESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>

#include <Vadon/Math/Rectangle.hpp>
namespace Vadon::Render
{
	class PipelineSystem : public GraphicsSystem<PipelineSystem>
	{
	public:
		virtual BlendStateHandle get_blend_state(const BlendInfo& blend_info) = 0;
		virtual bool is_blend_state_valid(BlendStateHandle blend_handle) const = 0;
		virtual void apply_blend_state(const BlendStateUpdate& blend_update) = 0;
		virtual void remove_blend_state(BlendStateHandle blend_handle) = 0;

		virtual RasterizerStateHandle get_rasterizer_state(const RasterizerInfo& rasterizer_info) = 0;
		virtual bool is_rasterizer_state_valid(RasterizerStateHandle rasterizer_handle) const = 0;
		virtual void apply_rasterizer_state(RasterizerStateHandle rasterizer_handle) = 0;
		virtual void remove_rasterizer_state(RasterizerStateHandle rasterizer_handle) = 0;

		virtual DepthStencilStateHandle get_depth_stencil_state(const DepthStencilInfo& depth_stencil_info) = 0;
		virtual bool is_depth_stencil_state_valid(DepthStencilStateHandle depth_stencil_handle) const = 0;
		virtual void apply_depth_stencil_state(const DepthStencilUpdate& depth_stencil_update) = 0;
		virtual void remove_depth_stencil_state(DepthStencilStateHandle depth_stencil_handle) = 0;

		virtual void set_primitive_topology(PrimitiveTopology topology) = 0;
		virtual void set_scissor(const Math::RectangleInt& scissor_rectangle) = 0;
	protected:
		PipelineSystem(Core::EngineCoreInterface& core) : 
			System(core) 
		{
		}
	};
}
#endif