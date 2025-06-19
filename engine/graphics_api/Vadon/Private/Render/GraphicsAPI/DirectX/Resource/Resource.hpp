#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_RESOURCE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Resource/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
namespace Vadon::Private::Render::DirectX
{
    using ResourceType = Vadon::Render::ResourceType;
    using ResourceUsage = Vadon::Render::ResourceUsage;
    using ResourceCPUAccessFlags = Vadon::Render::ResourceCPUAccessFlags;

    using ResourceHandle = Vadon::Render::ResourceHandle;

	template<typename T>
	using TypedResourceHandle = Vadon::Render::TypedResourceHandle<T>;

    using D3DResource = ComPtr<ID3D11Resource>;

    struct Resource
    {
		virtual ~Resource() {}

		ResourceType type;
        D3DResource d3d_resource;
		// TODO: any other data?
    };

	VADON_GRAPHICSAPI_DECLARE_TYPED_RESOURCE_HANDLE(Resource, D3DResourceHandle);

	constexpr D3D11_RESOURCE_DIMENSION get_d3d_resource_dimension(ResourceType resource_type)
	{
		switch (resource_type)
		{
		case ResourceType::UNKNOWN:
			return D3D11_RESOURCE_DIMENSION_UNKNOWN;
		case ResourceType::BUFFER:
			return D3D11_RESOURCE_DIMENSION_BUFFER;
		case ResourceType::TEXTURE_1D:
			return D3D11_RESOURCE_DIMENSION_TEXTURE1D;
		case ResourceType::TEXTURE_2D:
			return D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		case ResourceType::TEXTURE_3D:
			return D3D11_RESOURCE_DIMENSION_TEXTURE3D;
		}

		return D3D11_RESOURCE_DIMENSION_UNKNOWN;
	}

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