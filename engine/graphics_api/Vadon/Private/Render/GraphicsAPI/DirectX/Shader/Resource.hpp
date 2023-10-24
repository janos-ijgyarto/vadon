#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using ResourceType = Vadon::Render::ResourceType;
	using ResourceUsage = Vadon::Render::ResourceUsage;
	using ResourceBindFlags = Vadon::Render::ResourceBindFlags;
	using ResourceMiscFlags = Vadon::Render::ResourceMiscFlags;
	using ResourceAccessFlags = Vadon::Render::ResourceAccessFlags;

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

	constexpr D3D11_BIND_FLAG get_d3d_bind_flags(ResourceBindFlags bind_flags)
	{
		UINT d3d_bind_flags = 0;
		VADON_START_BITMASK_SWITCH(bind_flags)
		{
		case ResourceBindFlags::VERTEX_BUFFER:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			break;
		case ResourceBindFlags::INDEX_BUFFER:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			break;
		case ResourceBindFlags::CONSTANT_BUFFER:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
			break;
		case ResourceBindFlags::SHADER_RESOURCE:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
			break;
		case ResourceBindFlags::STREAM_OUTPUT:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_STREAM_OUTPUT;
			break;
		case ResourceBindFlags::RENDER_TARGET:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
			break;
		case ResourceBindFlags::DEPTH_STENCIL:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
			break;
		case ResourceBindFlags::UNORDERED_ACCESS:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
			break;
		case ResourceBindFlags::DECODER:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_DECODER;
			break;
		case ResourceBindFlags::VIDEO_ENCODER:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_VIDEO_ENCODER;
			break;
		}

		return D3D11_BIND_FLAG(d3d_bind_flags);
	}

	constexpr ResourceBindFlags get_resource_bind_flags(D3D11_BIND_FLAG d3d_bind_flags)
	{
		ResourceBindFlags bind_flags = ResourceBindFlags::NONE;

		VADON_START_BITMASK_SWITCH(d3d_bind_flags)
		{
		case D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER:
			bind_flags |= ResourceBindFlags::VERTEX_BUFFER;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER:
			bind_flags |= ResourceBindFlags::INDEX_BUFFER;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER:
			bind_flags |= ResourceBindFlags::CONSTANT_BUFFER;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_STREAM_OUTPUT:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_DECODER:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_VIDEO_ENCODER:
			bind_flags |= ResourceBindFlags::SHADER_RESOURCE;
			break;
		}

		return bind_flags;
	}

	constexpr D3D11_RESOURCE_MISC_FLAG get_d3d_misc_flags(ResourceMiscFlags flags)
	{
		UINT d3d_misc_flags = 0;
		VADON_START_BITMASK_SWITCH(flags)
		{
		case ResourceMiscFlags::GENERATE_MIPS:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			break;
		case ResourceMiscFlags::SHARED:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_SHARED;
			break;
		case ResourceMiscFlags::TEXTURECUBE:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			break;
		case ResourceMiscFlags::DRAWINDIRECT_ARGS:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			break;
		case ResourceMiscFlags::BUFFER_ALLOW_RAW_VIEWS:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			break;
		case ResourceMiscFlags::BUFFER_STRUCTURED:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;
		case ResourceMiscFlags::RESOURCE_CLAMP:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_RESOURCE_CLAMP;
			break;
		case ResourceMiscFlags::SHARED_KEYEDMUTEX:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
			break;
		case ResourceMiscFlags::GDI_COMPATIBLE:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
			break;
		case ResourceMiscFlags::SHARED_NTHANDLE: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
			break;
		case ResourceMiscFlags::RESTRICTED_CONTENT: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_RESTRICTED_CONTENT;
			break;
		case ResourceMiscFlags::RESTRICT_SHARED_RESOURCE: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE;
			break;
		case ResourceMiscFlags::RESTRICT_SHARED_RESOURCE_DRIVER: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER;
			break;
		case ResourceMiscFlags::GUARDED: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_GUARDED;
			break;
		case ResourceMiscFlags::TILE_POOL: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_TILE_POOL;
			break;
		case ResourceMiscFlags::TILED: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_TILED;
			break;
		case ResourceMiscFlags::HW_PROTECTED: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_HW_PROTECTED;
			break;
		case ResourceMiscFlags::SHARED_DISPLAYABLE: 
			d3d_misc_flags |= D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE;
			break;
		case ResourceMiscFlags::SHARED_EXCLUSIVE_WRITER:
			d3d_misc_flags |= D3D11_RESOURCE_MISC_SHARED_EXCLUSIVE_WRITER;
			break;
		}

		return D3D11_RESOURCE_MISC_FLAG(d3d_misc_flags);
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
		case ResourceType::BUFFEREX:
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
			return ResourceType::BUFFEREX;
		}

		return ResourceType::UNKNOWN;
	}
}
#endif