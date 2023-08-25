#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_PIPELINE_PIPELINESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_PIPELINE_PIPELINESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Pipeline/Pipeline.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class PipelineSystem final : public Vadon::Render::PipelineSystem
	{
		BlendStateHandle get_blend_state(const BlendInfo& blend_info) override;
		void apply_blend_state(const BlendStateUpdate& blend_update) override;
		void remove_blend_state(BlendStateHandle blend_handle) override;

		RasterizerStateHandle get_rasterizer_state(const RasterizerInfo& rasterizer_info) override;
		void apply_rasterizer_state(RasterizerStateHandle rasterizer_handle) override;
		void remove_rasterizer_state(RasterizerStateHandle rasterizer_handle) override;

		DepthStencilStateHandle get_depth_stencil_state(const DepthStencilInfo& depth_stencil_info) override;
		void apply_depth_stencil_state(const DepthStencilUpdate& depth_stencil_update) override;
		void remove_depth_stencil_state(DepthStencilStateHandle depth_stencil_handle) override;

		void set_primitive_topology(PrimitiveTopology topology) override;
		void set_scissor(const Vadon::Utilities::RectangleInt& scissor_rectangle) override;
	private:
		using BlendStatePool = Vadon::Utilities::ObjectPool<Vadon::Render::BlendState, BlendState>;
		using RasterizerStatePool = Vadon::Utilities::ObjectPool<Vadon::Render::RasterizerState, RasterizerState>;
		using DepthStencilStatePool = Vadon::Utilities::ObjectPool<Vadon::Render::DepthStencilState, DepthStencilState>;

		PipelineSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		BlendStatePool m_blend_state_pool;
		RasterizerStatePool m_rasterizer_state_pool;
		DepthStencilStatePool m_depth_stencil_state_pool;

		friend GraphicsAPI;
	};
}
#endif