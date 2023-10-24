#ifndef VADON_RENDER_GRAPHICSAPI_PIPELINE_PIPELINE_HPP
#define VADON_RENDER_GRAPHICSAPI_PIPELINE_PIPELINE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <array>
namespace Vadon::Render
{
	// NOTE: this is basically a thin wrapper around D3D11
	// FIXME: trim the features from this that are most likely not going to be used
	enum class BlendType
	{
		ZERO,
		ONE,
		SRC_COLOR,
		INV_SRC_COLOR,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DEST_ALPHA,
		INV_DEST_ALPHA,
		DEST_COLOR,
		INV_DEST_COLOR,
		SRC_ALPHA_SAT,
		BLEND_FACTOR,
		INV_BLEND_FACTOR,
		SRC1_COLOR,
		INV_SRC1_COLOR,
		SRC1_ALPHA,
		INV_SRC1_ALPHA
	};

	enum class BlendOperation
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX
	};

	enum class ColorWriteEnable : uint8_t
	{
		NONE = 0,
		RED = 1 << 0,
		GREEN = 1 << 1,
		BLUE = 1 << 2,
		ALPHA = 1 << 3,
		ALL = (((RED | GREEN) | BLUE) | ALPHA)
	};

	struct RenderTargetBlendInfo
	{
		bool blend_enable = false;
		BlendType source_blend = BlendType::ZERO;
		BlendType dest_blend = BlendType::ZERO;
		BlendOperation blend_operation = BlendOperation::ADD;
		BlendType source_blend_alpha = BlendType::ZERO;
		BlendType dest_blend_alpha = BlendType::ZERO;
		BlendOperation blend_operation_alpha = BlendOperation::ADD;
		ColorWriteEnable write_mask = ColorWriteEnable::NONE;
	};

	struct BlendInfo
	{
		bool alpha_to_coverage_enable = false;
		bool independent_blend_enable = false;
		std::array<RenderTargetBlendInfo, 8> render_target;
	};

	using BlendFactorArray = std::array<float, 4>;

	VADON_DECLARE_TYPED_POOL_HANDLE(BlendState, BlendStateHandle);

	struct BlendStateUpdate
	{
		BlendStateHandle blend_state;
		BlendFactorArray blend_factor = { 1.0f, 1.0f, 1.0f ,1.0f };
		uint32_t sample_mask = 0xffffffff;
	};

	enum class RasterizerFillMode
	{
		WIREFRAME,
		SOLID
	};

	enum class RasterizerCullMode
	{
		NONE,
		FRONT,
		BACK
	};

	struct RasterizerInfo
	{
		RasterizerFillMode fill_mode = RasterizerFillMode::SOLID;
		RasterizerCullMode cull_mode = RasterizerCullMode::NONE;
		bool front_counter_clockwise = false;
		int depth_bias = 0;
		float depth_bias_clamp = 0.0f;
		float slope_scaled_depth_bias = 0.0f;
		bool depth_clip_enable = false;
		bool scissor_enable = false;
		bool multisample_enable = false;
		bool antialiased_line_enable = false;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(RasterizerState, RasterizerStateHandle);

	enum class DepthWriteMask
	{
		ZERO = 0,
		ALL = 1
	};

	struct DepthInfo
	{
		bool enable = false;
		DepthWriteMask write_mask = DepthWriteMask::ZERO;
		GraphicsAPIComparisonFunction comparison_func = GraphicsAPIComparisonFunction::NEVER;
	};

	enum class StencilOperation
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_SAT,
		DECR_SAT,
		INVERT,
		INCR,
		DECR
	};

	struct StencilOperationInfo
	{
		StencilOperation fail = StencilOperation::KEEP;
		StencilOperation depth_fail = StencilOperation::KEEP;
		StencilOperation pass = StencilOperation::KEEP;
		GraphicsAPIComparisonFunction comparison_func = GraphicsAPIComparisonFunction::NEVER;
	};

	struct StencilInfo
	{
		bool enable = false;
		uint8_t read_mask = 0;
		uint8_t write_mask = 0;
		StencilOperationInfo front_face;
		StencilOperationInfo back_face;
	};

	struct DepthStencilInfo
	{
		DepthInfo depth;
		StencilInfo stencil;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(DepthStencilState, DepthStencilStateHandle);

	struct DepthStencilUpdate
	{
		DepthStencilStateHandle depth_stencil;
		uint32_t stencil_ref = 0;
	};

	struct PipelineState
	{
		BlendStateUpdate blend_update;
		RasterizerStateHandle rasterizer_state;
		DepthStencilUpdate depth_stencil_update;
	};

	enum class PrimitiveTopology
	{
		// TODO: other supported topologies?
		UNDEFINED,
		LINE_LIST,
		TRIANGLE_LIST
	};
}
#endif