#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_PIPELINE_PIPELINE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_PIPELINE_PIPELINE_HPP
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using BlendType = Vadon::Render::BlendType;
	using BlendOperation = Vadon::Render::BlendOperation;
	using ColorWriteEnable = Vadon::Render::ColorWriteEnable;
	using RenderTargetBlendInfo = Vadon::Render::RenderTargetBlendInfo;
	using BlendInfo = Vadon::Render::BlendInfo;

	using BlendStateHandle = Vadon::Render::BlendStateHandle;

	using BlendFactorArray = Vadon::Render::BlendFactorArray;
	using BlendStateUpdate = Vadon::Render::BlendStateUpdate;

	using D3DBlendState = ComPtr<ID3D11BlendState>;

	struct BlendState
	{
		BlendInfo info;
		D3DBlendState d3d_blend_state;
	};

	using RasterizerFillMode = Vadon::Render::RasterizerFillMode;
	using RasterizerCullMode = Vadon::Render::RasterizerCullMode;
	using RasterizerInfo = Vadon::Render::RasterizerInfo;

	using RasterizerStateHandle = Vadon::Render::RasterizerStateHandle;

	using D3DRasterizerState = ComPtr<ID3D11RasterizerState>;

	struct RasterizerState
	{
		RasterizerInfo info;
		D3DRasterizerState d3d_rasterizer_state;
	};

	using DepthWriteMask = Vadon::Render::DepthWriteMask;
	using StencilOperation = Vadon::Render::StencilOperation;
	using StencilOperationInfo = Vadon::Render::StencilOperationInfo;

	using DepthInfo = Vadon::Render::DepthInfo;
	using StencilInfo = Vadon::Render::StencilInfo;
	using DepthStencilInfo = Vadon::Render::DepthStencilInfo;

	using DepthStencilStateHandle = Vadon::Render::DepthStencilStateHandle;

	using DepthStencilUpdate = Vadon::Render::DepthStencilUpdate;

	using D3DDepthStencilState = ComPtr<ID3D11DepthStencilState>;

	struct DepthStencilState
	{
		DepthStencilInfo info;
		D3DDepthStencilState d3d_depth_stencil_state;
	};

	using PrimitiveTopology = Vadon::Render::PrimitiveTopology;
}
#endif