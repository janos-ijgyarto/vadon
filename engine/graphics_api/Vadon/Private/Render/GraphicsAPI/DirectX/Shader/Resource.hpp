#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using ResourceType = Vadon::Render::ResourceType;
	using ResourceUsage = Vadon::Render::ResourceUsage;
	using ResourceCPUAccessFlags = Vadon::Render::ResourceCPUAccessFlags;

	using ResourceTypeInfo = Vadon::Render::ResourceTypeInfo;
	using ResourceViewInfo = Vadon::Render::ResourceViewInfo;

	using ResourceViewHandle = Vadon::Render::ResourceViewHandle;

	using D3DResourcePtr = ID3D11Resource*;
	using D3DResource = ComPtr<ID3D11Resource>;
	using D3DShaderResourceView = ComPtr<ID3D11ShaderResourceView>;

	struct ShaderResourceView
	{
		Vadon::Render::ResourceViewInfo info;
		D3DShaderResourceView d3d_srv;
	};

	using D3DUnorderedAccessView = ComPtr<ID3D11UnorderedAccessView>;

	constexpr D3D11_USAGE get_d3d_usage(ResourceUsage usage)
	{
		switch (usage)
		{
		case ResourceUsage::DEFAULT:
			return D3D11_USAGE::D3D11_USAGE_DEFAULT;
		case ResourceUsage::IMMUTABLE:
			return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
		case ResourceUsage::DYNAMIC:
			return D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		case ResourceUsage::STAGING:
			return D3D11_USAGE::D3D11_USAGE_STAGING;
		}

		return D3D11_USAGE::D3D11_USAGE_DEFAULT;
	}

	constexpr ResourceUsage get_resource_usage(D3D11_USAGE usage)
	{
		switch (usage)
		{
		case D3D11_USAGE::D3D11_USAGE_DEFAULT:
			return ResourceUsage::DEFAULT;
		case D3D11_USAGE::D3D11_USAGE_IMMUTABLE:
			return ResourceUsage::IMMUTABLE;
		case D3D11_USAGE::D3D11_USAGE_DYNAMIC:
			return ResourceUsage::DYNAMIC;
		case D3D11_USAGE::D3D11_USAGE_STAGING:
			return ResourceUsage::STAGING;
		}

		return ResourceUsage::DEFAULT;
	}

	constexpr D3D11_SRV_DIMENSION get_d3d_srv_dimension(ResourceType resource_type)
	{
		switch (resource_type)
		{
		case ResourceType::UNKNOWN:
			return D3D11_SRV_DIMENSION_UNKNOWN;
		case ResourceType::BUFFER:
			return D3D11_SRV_DIMENSION_BUFFER;
		case ResourceType::TEXTURE_1D:
			return D3D11_SRV_DIMENSION_TEXTURE1D;
		case ResourceType::TEXTURE_1D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		case ResourceType::TEXTURE_2D:
			return D3D11_SRV_DIMENSION_TEXTURE2D;
		case ResourceType::TEXTURE_2D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		case ResourceType::TEXTURE_2D_MS:
			return D3D11_SRV_DIMENSION_TEXTURE2DMS;
		case ResourceType::TEXTURE_2D_MS_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		case ResourceType::TEXTURE_3D:
			return D3D11_SRV_DIMENSION_TEXTURE3D;
		case ResourceType::TEXTURE_CUBE:
			return D3D11_SRV_DIMENSION_TEXTURECUBE;
		case ResourceType::TEXTURE_CUBE_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		case ResourceType::RAW_BUFFER:
			return D3D11_SRV_DIMENSION_BUFFEREX;
		}

		return D3D11_SRV_DIMENSION_UNKNOWN;
	}

	constexpr ResourceType get_srv_type(D3D11_SRV_DIMENSION srv_dimension)
	{
		switch (srv_dimension)
		{
		case D3D11_SRV_DIMENSION_UNKNOWN:
			return ResourceType::UNKNOWN;
		case D3D11_SRV_DIMENSION_BUFFER:
			return ResourceType::BUFFER;
		case D3D11_SRV_DIMENSION_TEXTURE1D:
			return ResourceType::TEXTURE_1D;
		case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
			return ResourceType::TEXTURE_1D_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE2D:
			return ResourceType::TEXTURE_2D;
		case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
			return ResourceType::TEXTURE_2D_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE2DMS:
			return ResourceType::TEXTURE_2D_MS;
		case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
			return ResourceType::TEXTURE_2D_MS_ARRAY;
		case D3D11_SRV_DIMENSION_TEXTURE3D:
			return ResourceType::TEXTURE_3D;
		case D3D11_SRV_DIMENSION_TEXTURECUBE:
			return ResourceType::TEXTURE_CUBE;
		case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
			return ResourceType::TEXTURE_CUBE_ARRAY;
		case D3D11_SRV_DIMENSION_BUFFEREX:
			return ResourceType::RAW_BUFFER;
		}

		return ResourceType::UNKNOWN;
	}

	constexpr D3D11_CPU_ACCESS_FLAG get_d3d_cpu_access_flags(ResourceCPUAccessFlags access_flags)
	{
		UINT cpu_access_flags = 0;

		VADON_START_BITMASK_SWITCH(access_flags)
		{
		case ResourceCPUAccessFlags::WRITE:
			cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
			break;
		case ResourceCPUAccessFlags::READ:
			cpu_access_flags |= D3D11_CPU_ACCESS_READ;
			break;
		}

		return D3D11_CPU_ACCESS_FLAG(cpu_access_flags);
	}

	constexpr ResourceCPUAccessFlags get_cpu_access_flags(D3D11_CPU_ACCESS_FLAG access_flags)
	{
		ResourceCPUAccessFlags cpu_access_flags = ResourceCPUAccessFlags::NONE;

		VADON_START_BITMASK_SWITCH(access_flags)
		{
		case D3D11_CPU_ACCESS_WRITE:
			cpu_access_flags |= ResourceCPUAccessFlags::WRITE;
			break;
		case D3D11_CPU_ACCESS_READ:
			cpu_access_flags |= ResourceCPUAccessFlags::READ;
			break;
		}

		return cpu_access_flags;
	}
}
#endif