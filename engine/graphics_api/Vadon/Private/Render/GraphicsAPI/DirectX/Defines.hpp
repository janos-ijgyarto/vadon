#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_DEFINES_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_DEFINES_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using GraphicsAPIDataFormat = Vadon::Render::GraphicsAPIDataFormat;
	using GraphicsAPIComparisonFunction = Vadon::Render::GraphicsAPIComparisonFunction;

	constexpr DXGI_FORMAT get_dxgi_format(GraphicsAPIDataFormat format)
	{
		switch (format)
		{
		case GraphicsAPIDataFormat::UNKNOWN:
			return DXGI_FORMAT_UNKNOWN;
		case GraphicsAPIDataFormat::FLOAT4_32:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case GraphicsAPIDataFormat::FLOAT2_32:
			return DXGI_FORMAT_R32G32_FLOAT;
		case GraphicsAPIDataFormat::UNORM4_8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GraphicsAPIDataFormat::FLOAT_32:
			return DXGI_FORMAT_R32_FLOAT;
		case GraphicsAPIDataFormat::UINT_32:
			return DXGI_FORMAT_R32_UINT;
		case GraphicsAPIDataFormat::UINT_16:
			return DXGI_FORMAT_R16_UINT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	constexpr D3D11_COMPARISON_FUNC get_d3d_comparison_func(GraphicsAPIComparisonFunction func)
	{
		switch (func)
		{
		case GraphicsAPIComparisonFunction::NEVER:
			return D3D11_COMPARISON_NEVER;
		case GraphicsAPIComparisonFunction::LESS:
			return D3D11_COMPARISON_LESS;
		case GraphicsAPIComparisonFunction::EQUAL:
			return D3D11_COMPARISON_EQUAL;
		case GraphicsAPIComparisonFunction::LESS_EQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
		case GraphicsAPIComparisonFunction::GREATER:
			return D3D11_COMPARISON_GREATER;
		case GraphicsAPIComparisonFunction::NOT_EQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
		case GraphicsAPIComparisonFunction::GREATER_EQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case GraphicsAPIComparisonFunction::ALWAYS:
			return D3D11_COMPARISON_ALWAYS;
		}

		return D3D11_COMPARISON_ALWAYS;
	}
}
#endif