#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_PIPELINE_PIPELINESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_PIPELINE_PIPELINESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	class PipelineSystem final : public Vadon::Render::PipelineSystem
	{
	public:
		Vadon::Render::BlendStateHandle get_blend_state(const Vadon::Render::BlendInfo& blend_info) override;
		bool is_blend_state_valid(Vadon::Render::BlendStateHandle /*blend_handle*/) const override { return false; }
		void apply_blend_state(const Vadon::Render::BlendStateUpdate& blend_update) override;
		void remove_blend_state(Vadon::Render::BlendStateHandle blend_handle) override;

		Vadon::Render::RasterizerStateHandle get_rasterizer_state(const Vadon::Render::RasterizerInfo& rasterizer_info) override;
		bool is_rasterizer_state_valid(Vadon::Render::RasterizerStateHandle /*rasterizer_handle*/) const override { return false; }
		void apply_rasterizer_state(Vadon::Render::RasterizerStateHandle rasterizer_handle) override;
		void remove_rasterizer_state(Vadon::Render::RasterizerStateHandle rasterizer_handle) override;

		Vadon::Render::DepthStencilStateHandle get_depth_stencil_state(const Vadon::Render::DepthStencilInfo& depth_stencil_info) override;
		bool is_depth_stencil_state_valid(Vadon::Render::DepthStencilStateHandle /*depth_stencil_handle*/) const override { return false; }
		void apply_depth_stencil_state(const Vadon::Render::DepthStencilUpdate& depth_stencil_update) override;
		void remove_depth_stencil_state(Vadon::Render::DepthStencilStateHandle depth_stencil_handle) override;

		void set_primitive_topology(Vadon::Render::PrimitiveTopology topology) override;
		void set_scissor(const Vadon::Utilities::RectangleInt& scissor_rectangle) override;
	private:
		PipelineSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif