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
        case GraphicsAPIDataFormat::BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

    constexpr GraphicsAPIDataFormat get_graphics_api_data_format(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_UNKNOWN:
            return GraphicsAPIDataFormat::UNKNOWN;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return GraphicsAPIDataFormat::FLOAT4_32;
        case DXGI_FORMAT_R32G32_FLOAT:
            return GraphicsAPIDataFormat::FLOAT2_32;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return GraphicsAPIDataFormat::UNORM4_8;
        case DXGI_FORMAT_R32_FLOAT:
            return GraphicsAPIDataFormat::FLOAT_32;
        case DXGI_FORMAT_R32_UINT:
            return GraphicsAPIDataFormat::UINT_32;
        case DXGI_FORMAT_R16_UINT:
            return GraphicsAPIDataFormat::UINT_16;
        case DXGI_FORMAT_BC1_UNORM:
            return GraphicsAPIDataFormat::BC1_UNORM;
        }

        return GraphicsAPIDataFormat::UNKNOWN;
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

	constexpr size_t get_bits_per_pixel(DXGI_FORMAT dxgi_format) noexcept
    {
        switch (dxgi_format)
        {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        default:
            return 0;
        }
    }
}
#endif