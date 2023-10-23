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
        constexpr DXGI_FORMAT c_dxgi_lookup[] = {
            DXGI_FORMAT_UNKNOWN,
            DXGI_FORMAT_R32G32B32A32_TYPELESS,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_R32G32B32A32_UINT,
            DXGI_FORMAT_R32G32B32A32_SINT,
            DXGI_FORMAT_R32G32B32_TYPELESS,
            DXGI_FORMAT_R32G32B32_FLOAT,
            DXGI_FORMAT_R32G32B32_UINT,
            DXGI_FORMAT_R32G32B32_SINT,
            DXGI_FORMAT_R16G16B16A16_TYPELESS,
            DXGI_FORMAT_R16G16B16A16_FLOAT,
            DXGI_FORMAT_R16G16B16A16_UNORM,
            DXGI_FORMAT_R16G16B16A16_UINT,
            DXGI_FORMAT_R16G16B16A16_SNORM,
            DXGI_FORMAT_R16G16B16A16_SINT,
            DXGI_FORMAT_R32G32_TYPELESS,
            DXGI_FORMAT_R32G32_FLOAT,
            DXGI_FORMAT_R32G32_UINT,
            DXGI_FORMAT_R32G32_SINT,
            DXGI_FORMAT_R32G8X24_TYPELESS,
            DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
            DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
            DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
            DXGI_FORMAT_R10G10B10A2_TYPELESS,
            DXGI_FORMAT_R10G10B10A2_UNORM,
            DXGI_FORMAT_R10G10B10A2_UINT,
            DXGI_FORMAT_R11G11B10_FLOAT,
            DXGI_FORMAT_R8G8B8A8_TYPELESS,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            DXGI_FORMAT_R8G8B8A8_UINT,
            DXGI_FORMAT_R8G8B8A8_SNORM,
            DXGI_FORMAT_R8G8B8A8_SINT,
            DXGI_FORMAT_R16G16_TYPELESS,
            DXGI_FORMAT_R16G16_FLOAT,
            DXGI_FORMAT_R16G16_UNORM,
            DXGI_FORMAT_R16G16_UINT,
            DXGI_FORMAT_R16G16_SNORM,
            DXGI_FORMAT_R16G16_SINT,
            DXGI_FORMAT_R32_TYPELESS,
            DXGI_FORMAT_D32_FLOAT,
            DXGI_FORMAT_R32_FLOAT,
            DXGI_FORMAT_R32_UINT,
            DXGI_FORMAT_R32_SINT,
            DXGI_FORMAT_R24G8_TYPELESS,
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
            DXGI_FORMAT_X24_TYPELESS_G8_UINT,
            DXGI_FORMAT_R8G8_TYPELESS,
            DXGI_FORMAT_R8G8_UNORM,
            DXGI_FORMAT_R8G8_UINT,
            DXGI_FORMAT_R8G8_SNORM,
            DXGI_FORMAT_R8G8_SINT,
            DXGI_FORMAT_R16_TYPELESS,
            DXGI_FORMAT_R16_FLOAT,
            DXGI_FORMAT_D16_UNORM,
            DXGI_FORMAT_R16_UNORM,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R16_SNORM,
            DXGI_FORMAT_R16_SINT,
            DXGI_FORMAT_R8_TYPELESS,
            DXGI_FORMAT_R8_UNORM,
            DXGI_FORMAT_R8_UINT,
            DXGI_FORMAT_R8_SNORM,
            DXGI_FORMAT_R8_SINT,
            DXGI_FORMAT_A8_UNORM,
            DXGI_FORMAT_R1_UNORM,
            DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
            DXGI_FORMAT_R8G8_B8G8_UNORM,
            DXGI_FORMAT_G8R8_G8B8_UNORM,
            DXGI_FORMAT_BC1_TYPELESS,
            DXGI_FORMAT_BC1_UNORM,
            DXGI_FORMAT_BC1_UNORM_SRGB,
            DXGI_FORMAT_BC2_TYPELESS,
            DXGI_FORMAT_BC2_UNORM,
            DXGI_FORMAT_BC2_UNORM_SRGB,
            DXGI_FORMAT_BC3_TYPELESS,
            DXGI_FORMAT_BC3_UNORM,
            DXGI_FORMAT_BC3_UNORM_SRGB,
            DXGI_FORMAT_BC4_TYPELESS,
            DXGI_FORMAT_BC4_UNORM,
            DXGI_FORMAT_BC4_SNORM,
            DXGI_FORMAT_BC5_TYPELESS,
            DXGI_FORMAT_BC5_UNORM,
            DXGI_FORMAT_BC5_SNORM,
            DXGI_FORMAT_B5G6R5_UNORM,
            DXGI_FORMAT_B5G5R5A1_UNORM,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT_B8G8R8X8_UNORM,
            DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
            DXGI_FORMAT_B8G8R8A8_TYPELESS,
            DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            DXGI_FORMAT_B8G8R8X8_TYPELESS,
            DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
            DXGI_FORMAT_BC6H_TYPELESS,
            DXGI_FORMAT_BC6H_UF16,
            DXGI_FORMAT_BC6H_SF16,
            DXGI_FORMAT_BC7_TYPELESS,
            DXGI_FORMAT_BC7_UNORM,
            DXGI_FORMAT_BC7_UNORM_SRGB,
            DXGI_FORMAT_AYUV,
            DXGI_FORMAT_Y410,
            DXGI_FORMAT_Y416,
            DXGI_FORMAT_NV12,
            DXGI_FORMAT_P010,
            DXGI_FORMAT_P016,
            DXGI_FORMAT_420_OPAQUE,
            DXGI_FORMAT_YUY2,
            DXGI_FORMAT_Y210,
            DXGI_FORMAT_Y216,
            DXGI_FORMAT_NV11,
            DXGI_FORMAT_AI44,
            DXGI_FORMAT_IA44,
            DXGI_FORMAT_P8,
            DXGI_FORMAT_A8P8,
            DXGI_FORMAT_B4G4R4A4_UNORM,
            DXGI_FORMAT_P208,
            DXGI_FORMAT_V208,
            DXGI_FORMAT_V408,
            DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
            DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
            DXGI_FORMAT_FORCE_UINT
        };

        return c_dxgi_lookup[static_cast<size_t>(format)];
	}

    constexpr GraphicsAPIDataFormat get_graphics_api_data_format(DXGI_FORMAT format)
    {
        constexpr GraphicsAPIDataFormat c_format_lookup[] = {
            GraphicsAPIDataFormat::UNKNOWN,
            GraphicsAPIDataFormat::R32G32B32A32_TYPELESS,
            GraphicsAPIDataFormat::R32G32B32A32_FLOAT,
            GraphicsAPIDataFormat::R32G32B32A32_UINT,
            GraphicsAPIDataFormat::R32G32B32A32_SINT,
            GraphicsAPIDataFormat::R32G32B32_TYPELESS,
            GraphicsAPIDataFormat::R32G32B32_FLOAT,
            GraphicsAPIDataFormat::R32G32B32_UINT,
            GraphicsAPIDataFormat::R32G32B32_SINT,
            GraphicsAPIDataFormat::R16G16B16A16_TYPELESS,
            GraphicsAPIDataFormat::R16G16B16A16_FLOAT,
            GraphicsAPIDataFormat::R16G16B16A16_UNORM,
            GraphicsAPIDataFormat::R16G16B16A16_UINT,
            GraphicsAPIDataFormat::R16G16B16A16_SNORM,
            GraphicsAPIDataFormat::R16G16B16A16_SINT,
            GraphicsAPIDataFormat::R32G32_TYPELESS,
            GraphicsAPIDataFormat::R32G32_FLOAT,
            GraphicsAPIDataFormat::R32G32_UINT,
            GraphicsAPIDataFormat::R32G32_SINT,
            GraphicsAPIDataFormat::R32G8X24_TYPELESS,
            GraphicsAPIDataFormat::D32_FLOAT_S8X24_UINT,
            GraphicsAPIDataFormat::R32_FLOAT_X8X24_TYPELESS,
            GraphicsAPIDataFormat::X32_TYPELESS_G8X24_UINT,
            GraphicsAPIDataFormat::R10G10B10A2_TYPELESS,
            GraphicsAPIDataFormat::R10G10B10A2_UNORM,
            GraphicsAPIDataFormat::R10G10B10A2_UINT,
            GraphicsAPIDataFormat::R11G11B10_FLOAT,
            GraphicsAPIDataFormat::R8G8B8A8_TYPELESS,
            GraphicsAPIDataFormat::R8G8B8A8_UNORM,
            GraphicsAPIDataFormat::R8G8B8A8_UNORM_SRGB,
            GraphicsAPIDataFormat::R8G8B8A8_UINT,
            GraphicsAPIDataFormat::R8G8B8A8_SNORM,
            GraphicsAPIDataFormat::R8G8B8A8_SINT,
            GraphicsAPIDataFormat::R16G16_TYPELESS,
            GraphicsAPIDataFormat::R16G16_FLOAT,
            GraphicsAPIDataFormat::R16G16_UNORM,
            GraphicsAPIDataFormat::R16G16_UINT,
            GraphicsAPIDataFormat::R16G16_SNORM,
            GraphicsAPIDataFormat::R16G16_SINT,
            GraphicsAPIDataFormat::R32_TYPELESS,
            GraphicsAPIDataFormat::D32_FLOAT,
            GraphicsAPIDataFormat::R32_FLOAT,
            GraphicsAPIDataFormat::R32_UINT,
            GraphicsAPIDataFormat::R32_SINT,
            GraphicsAPIDataFormat::R24G8_TYPELESS,
            GraphicsAPIDataFormat::D24_UNORM_S8_UINT,
            GraphicsAPIDataFormat::R24_UNORM_X8_TYPELESS,
            GraphicsAPIDataFormat::X24_TYPELESS_G8_UINT,
            GraphicsAPIDataFormat::R8G8_TYPELESS,
            GraphicsAPIDataFormat::R8G8_UNORM,
            GraphicsAPIDataFormat::R8G8_UINT,
            GraphicsAPIDataFormat::R8G8_SNORM,
            GraphicsAPIDataFormat::R8G8_SINT,
            GraphicsAPIDataFormat::R16_TYPELESS,
            GraphicsAPIDataFormat::R16_FLOAT,
            GraphicsAPIDataFormat::D16_UNORM,
            GraphicsAPIDataFormat::R16_UNORM,
            GraphicsAPIDataFormat::R16_UINT,
            GraphicsAPIDataFormat::R16_SNORM,
            GraphicsAPIDataFormat::R16_SINT,
            GraphicsAPIDataFormat::R8_TYPELESS,
            GraphicsAPIDataFormat::R8_UNORM,
            GraphicsAPIDataFormat::R8_UINT,
            GraphicsAPIDataFormat::R8_SNORM,
            GraphicsAPIDataFormat::R8_SINT,
            GraphicsAPIDataFormat::A8_UNORM,
            GraphicsAPIDataFormat::R1_UNORM,
            GraphicsAPIDataFormat::R9G9B9E5_SHAREDEXP,
            GraphicsAPIDataFormat::R8G8_B8G8_UNORM,
            GraphicsAPIDataFormat::G8R8_G8B8_UNORM,
            GraphicsAPIDataFormat::BC1_TYPELESS,
            GraphicsAPIDataFormat::BC1_UNORM,
            GraphicsAPIDataFormat::BC1_UNORM_SRGB,
            GraphicsAPIDataFormat::BC2_TYPELESS,
            GraphicsAPIDataFormat::BC2_UNORM,
            GraphicsAPIDataFormat::BC2_UNORM_SRGB,
            GraphicsAPIDataFormat::BC3_TYPELESS,
            GraphicsAPIDataFormat::BC3_UNORM,
            GraphicsAPIDataFormat::BC3_UNORM_SRGB,
            GraphicsAPIDataFormat::BC4_TYPELESS,
            GraphicsAPIDataFormat::BC4_UNORM,
            GraphicsAPIDataFormat::BC4_SNORM,
            GraphicsAPIDataFormat::BC5_TYPELESS,
            GraphicsAPIDataFormat::BC5_UNORM,
            GraphicsAPIDataFormat::BC5_SNORM,
            GraphicsAPIDataFormat::B5G6R5_UNORM,
            GraphicsAPIDataFormat::B5G5R5A1_UNORM,
            GraphicsAPIDataFormat::B8G8R8A8_UNORM,
            GraphicsAPIDataFormat::B8G8R8X8_UNORM,
            GraphicsAPIDataFormat::R10G10B10_XR_BIAS_A2_UNORM,
            GraphicsAPIDataFormat::B8G8R8A8_TYPELESS,
            GraphicsAPIDataFormat::B8G8R8A8_UNORM_SRGB,
            GraphicsAPIDataFormat::B8G8R8X8_TYPELESS,
            GraphicsAPIDataFormat::B8G8R8X8_UNORM_SRGB,
            GraphicsAPIDataFormat::BC6H_TYPELESS,
            GraphicsAPIDataFormat::BC6H_UF16,
            GraphicsAPIDataFormat::BC6H_SF16,
            GraphicsAPIDataFormat::BC7_TYPELESS,
            GraphicsAPIDataFormat::BC7_UNORM,
            GraphicsAPIDataFormat::BC7_UNORM_SRGB,
            GraphicsAPIDataFormat::AYUV,
            GraphicsAPIDataFormat::Y410,
            GraphicsAPIDataFormat::Y416,
            GraphicsAPIDataFormat::NV12,
            GraphicsAPIDataFormat::P010,
            GraphicsAPIDataFormat::P016,
            GraphicsAPIDataFormat::_420_OPAQUE,
            GraphicsAPIDataFormat::YUY2,
            GraphicsAPIDataFormat::Y210,
            GraphicsAPIDataFormat::Y216,
            GraphicsAPIDataFormat::NV11,
            GraphicsAPIDataFormat::AI44,
            GraphicsAPIDataFormat::IA44,
            GraphicsAPIDataFormat::P8,
            GraphicsAPIDataFormat::A8P8,
            GraphicsAPIDataFormat::B4G4R4A4_UNORM
        };

        if (format <= DXGI_FORMAT_B4G4R4A4_UNORM)
        {
            return c_format_lookup[format];
        }
        else
        {
            switch (format)
            {
            case DXGI_FORMAT_P208:
                return GraphicsAPIDataFormat::P208;
            case DXGI_FORMAT_V208:
                return GraphicsAPIDataFormat::V208;
            case DXGI_FORMAT_V408:
                return GraphicsAPIDataFormat::V408;
            case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
                return GraphicsAPIDataFormat::SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
            case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
                return GraphicsAPIDataFormat::SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;
            case DXGI_FORMAT_FORCE_UINT:
                return GraphicsAPIDataFormat::FORCE_UINT;
            }
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