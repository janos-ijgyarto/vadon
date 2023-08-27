#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	using ShaderResourceType = Vadon::Render::ShaderResourceType;
	using ShaderResourceTypeData = Vadon::Render::ShaderResourceTypeData;
	using ShaderResourceViewInfo = Vadon::Render::ShaderResourceViewInfo;

	using ShaderResourceViewHandle = Vadon::Render::ShaderResourceViewHandle;

	using D3DResourcePtr = ID3D11Resource*;
	using D3DResource = ComPtr<ID3D11Resource>;
	using D3DShaderResourceView = ComPtr<ID3D11ShaderResourceView>;

	struct ShaderResourceView
	{
		Vadon::Render::ShaderResourceViewInfo info;
		D3DShaderResourceView d3d_srv;
	};

	using D3DUnorderedAccessView = ComPtr<ID3D11UnorderedAccessView>;

	constexpr D3D11_SRV_DIMENSION get_d3d_srv_dimension(ShaderResourceType resource_type)
	{
		switch (resource_type)
		{
		case ShaderResourceType::UNKNOWN:
			return D3D11_SRV_DIMENSION_UNKNOWN;
		case ShaderResourceType::BUFFER:
			return D3D11_SRV_DIMENSION_BUFFER;
		case ShaderResourceType::TEXTURE_1D:
			return D3D11_SRV_DIMENSION_TEXTURE1D;
		case ShaderResourceType::TEXTURE_1D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		case ShaderResourceType::TEXTURE_2D:
			return D3D11_SRV_DIMENSION_TEXTURE2D;
		case ShaderResourceType::TEXTURE_2D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		case ShaderResourceType::TEXTURE_2D_MS:
			return D3D11_SRV_DIMENSION_TEXTURE2DMS;
		case ShaderResourceType::TEXTURE_2D_MS_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		case ShaderResourceType::TEXTURE_3D:
			return D3D11_SRV_DIMENSION_TEXTURE3D;
		case ShaderResourceType::TEXTURE_CUBE:
			return D3D11_SRV_DIMENSION_TEXTURECUBE;
		case ShaderResourceType::TEXTURE_CUBE_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		case ShaderResourceType::BUFFEREX:
			return D3D11_SRV_DIMENSION_BUFFEREX;
		}

		return D3D11_SRV_DIMENSION_UNKNOWN;
	}

	constexpr ShaderResourceType get_srv_type(D3D11_SRV_DIMENSION srv_dimension)
	{
		switch (srv_dimension)
		{
		case D3D11_SRV_DIMENSION_UNKNOWN:
			return ShaderResourceType::UNKNOWN;
		case D3D11_SRV_DIMENSION_BUFFER:
			return ShaderResourceType::BUFFER;
		case D3D11_SRV_DIMENSION_TEXTURE1D:
			return ShaderResourceType::TEXTURE_1D;
		case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
			return ShaderResourceType::TEXTURE_1D_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE2D:
			return ShaderResourceType::TEXTURE_2D;
		case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
			return ShaderResourceType::TEXTURE_2D_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE2DMS:
			return ShaderResourceType::TEXTURE_2D_MS;
		case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
			return ShaderResourceType::TEXTURE_2D_MS_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE3D:
			return ShaderResourceType::TEXTURE_3D;
		case D3D11_SRV_DIMENSION_TEXTURECUBE:
			return ShaderResourceType::TEXTURE_CUBE;
		case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
			return ShaderResourceType::TEXTURE_CUBE_ARRAY;
		case D3D11_SRV_DIMENSION_BUFFEREX:
			return ShaderResourceType::BUFFEREX;
		}

		return ShaderResourceType::UNKNOWN;
	}
}
#endif