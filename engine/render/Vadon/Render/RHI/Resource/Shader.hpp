#ifndef VADON_RENDER_RHI_RESOURCE_SHADER_HPP
#define VADON_RENDER_RHI_RESOURCE_SHADER_HPP
#include <Vadon/Render/RHI/Common/Format.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <span>
namespace Vadon::Render::RHI
{
	enum class ShaderStage
	{
		ALL = 0,
		NONE = 0x7FFFFFFF,
		VERTEX_STAGE = 1 << 0,
		PIXEL_STAGE = 1 << 1,
		COMPUTE_STAGE = 1 << 2
	};

	struct ShaderStageDescription
	{
		ShaderStage stage = ShaderStage::ALL;
		std::span<char> bytecode;
	};

	enum class RasterizerFillMode : uint8_t
	{
		WIREFRAME,
		SOLID
	};

	enum class RasterizerCullMode : uint8_t
	{
		NONE,
		FRONT,
		BACK
	};

	struct DepthBiasDescription
	{
		float constant;
		float clamp;
		float slope;
	};

	struct ShaderRasterizerInfo
	{
		DepthBiasDescription depth_bias;
		RasterizerFillMode fill_mode;
		RasterizerCullMode cull_mode;
		bool front_counter_clockwise;
		bool depth_clamp;
		// TODO: additional params
	};

	enum class BlendFactor : uint8_t
	{
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA
	};

	enum class BlendOperation : uint8_t
	{
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX
	};

	enum class BlendColorWriteMask : uint8_t
	{
		NONE = 0,
		COLOR_R = 1 << 0,
		COLOR_G = 1 << 1,
		COLOR_B = 1 << 2,
		COLOR_A = 1 << 3,
	};

	struct ShaderBlendInfo
	{
		BlendFactor source_factor = BlendFactor::ZERO;
		BlendFactor dest_factor = BlendFactor::ZERO;
		BlendOperation operation = BlendOperation::ADD;
	};

	struct ShaderRenderTargetInfo
	{
		Format format = Format::UNKNOWN;
		ShaderBlendInfo color_blend;
		ShaderBlendInfo alpha_blend;
		BlendColorWriteMask write_mask = BlendColorWriteMask::NONE;
		bool blend_enabled = false;
	};

	enum class CompareOperation : uint8_t
	{
		NONE,
		ALWAYS,
		NEVER,
		EQUAL,
		NOT_EQUAL,
		LESS,
		LESS_EQUAL,
		GREATER,
		GREATER_EQUAL
	};

	struct ShaderDepthInfo
	{
		CompareOperation compare_op = CompareOperation::NONE;
		bool write = false;
		bool bounds_test = false;
	};

	enum class StencilOperation : uint8_t
	{
		KEEP,
		ZERO,
		REPLACE,
		INCREMENT_AND_CLAMP,
		DECREMENT_AND_CLAMP,
		INVERT,
		INCREMENT_AND_WRAP,
		DECREMENT_AND_WRAP
	};

	struct StencilInfo
	{
		CompareOperation compare_op;
		StencilOperation fail_op;
		StencilOperation pass_op;
		StencilOperation depth_fail_op;
		uint8_t write_mask;
		uint8_t compare_mask;
	};

	struct ShaderStencilInfo
	{
		StencilInfo front = {};
		StencilInfo back = {};
	};

	enum class LogicOp : uint8_t
	{
		NONE,
		CLEAR,
		AND,
		AND_REVERSE,
		COPY,
		AND_INVERTED,
		XOR,
		OR,
		NOR,
		EQUIVALENT,
		INVERT,
		OR_REVERSE,
		COPY_INVERTED,
		OR_INVERTED,
		NAND,
		SET
	};

	struct ShaderOutputMergerInfo
	{
		std::span<ShaderRenderTargetInfo> render_targets;
		ShaderDepthInfo depth;
		ShaderStencilInfo stencil;
		Format depth_stencil_format;
		LogicOp logic_op;
	};
		
	struct GraphicsShaderDescription
	{
		ShaderRasterizerInfo rasterizer;
		ShaderOutputMergerInfo output_merger;
		std::span<ShaderStageDescription> stages;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(GraphicsShader, GraphicsShaderHandle);

	struct ComputeShaderDescription
	{
		// TODO!
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(ComputeShader, ComputeShaderHandle);
}
#endif