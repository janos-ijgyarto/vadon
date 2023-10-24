#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/TextureSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/Buffer.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/DDSTextureLoader11.h>

namespace Vadon::Private::Render::DirectX
{
	namespace
	{
		constexpr D3D11_FILTER get_d3d_filter(TextureFilter filter)
		{
			// FIXME: implement proper mapping for all types
			switch (filter)
			{
			case TextureFilter::MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MIN_MAG_MIP_POINT;
			case TextureFilter::MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}

			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

		constexpr D3D11_TEXTURE_ADDRESS_MODE get_d3d_texture_address_mode(TextureAddressMode address_mode)
		{
			switch (address_mode)
			{
			case TextureAddressMode::WRAP:
				return D3D11_TEXTURE_ADDRESS_WRAP;
			case TextureAddressMode::MIRROR:
				return D3D11_TEXTURE_ADDRESS_MIRROR;
			case TextureAddressMode::CLAMP:
				return D3D11_TEXTURE_ADDRESS_CLAMP;
			case TextureAddressMode::BORDER:
				return D3D11_TEXTURE_ADDRESS_BORDER;
			case TextureAddressMode::MIRROR_ONCE:
				return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

			}

			return D3D11_TEXTURE_ADDRESS_WRAP;
		}

		D3D11_TEXTURE2D_DESC create_2d_texture_desc(const TextureInfo& texture_info)
		{
			D3D11_TEXTURE2D_DESC texture_description;
			ZeroMemory(&texture_description, sizeof(D3D11_TEXTURE2D_DESC));

			texture_description.Width = texture_info.dimensions.x;
			texture_description.Height = texture_info.dimensions.y;
			texture_description.MipLevels = texture_info.mip_levels;
			texture_description.ArraySize = texture_info.array_size;
			texture_description.Format = get_dxgi_format(texture_info.format);

			texture_description.SampleDesc.Count = texture_info.sample_info.count;
			texture_description.SampleDesc.Quality = texture_info.sample_info.quality;

			texture_description.Usage = get_d3d_usage(texture_info.usage);
			texture_description.BindFlags = get_d3d_bind_flags(texture_info.bind_flags);

			// FIXME: use access flags from the info struct?
			if (texture_info.usage == ResourceUsage::DYNAMIC)
			{
				// Dynamic textures need write access
				texture_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}

			texture_description.MiscFlags = get_d3d_misc_flags(texture_info.misc);

			return texture_description;
		}

		TextureInfo create_texture_info(const D3DResource& texture_resource)
		{
			D3D11_RESOURCE_DIMENSION resource_dimension;
			texture_resource->GetType(&resource_dimension);

			TextureInfo texture_info;

			switch (resource_dimension)
			{
			case D3D11_RESOURCE_DIMENSION_UNKNOWN:
				// TODO!!!
				break;
			case D3D11_RESOURCE_DIMENSION_BUFFER:
				// TODO!!!
				break;
			case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
				// TODO!!!
				break;
			case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3DTexture2D d3d_texture_2d;
				HRESULT result = texture_resource.As(&d3d_texture_2d);
				if (FAILED(result))
				{
					return texture_info;
				}

				D3D11_TEXTURE2D_DESC texture_description;
				ZeroMemory(&texture_description, sizeof(D3D11_TEXTURE2D_DESC));

				d3d_texture_2d->GetDesc(&texture_description);

				texture_info.dimensions.x = texture_description.Width;
				texture_info.dimensions.y = texture_description.Height;
				texture_info.mip_levels = texture_description.MipLevels;
				texture_info.array_size = texture_description.ArraySize;
				texture_info.format = get_graphics_api_data_format(texture_description.Format);

				texture_info.sample_info.count = texture_description.SampleDesc.Count;
				texture_info.sample_info.quality = texture_description.SampleDesc.Quality;

				texture_info.usage = get_resource_usage(texture_description.Usage);
				texture_info.bind_flags = get_resource_bind_flags(Utilities::to_enum<D3D11_BIND_FLAG>(texture_description.BindFlags));
			}
			break;
			case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
				// TODO!!!
				break;
			}

			return texture_info;
		}
	}

	TextureHandle TextureSystem::create_texture_from_memory(size_t size, const void* data)
	{
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		
		D3DResource texture_resource;

		// TODO: could just move code into this function to fill in the texture info
		HRESULT result = ::DirectX::CreateDDSTextureFromMemory(device, static_cast<const uint8_t*>(data), size, texture_resource.ReleaseAndGetAddressOf(), nullptr, 0, nullptr);
		if (FAILED(result))
		{
			return TextureHandle();
		}

		// Make sure we can query valid texture info
		const TextureInfo texture_info = create_texture_info(texture_resource);
		if (texture_info.is_valid() == false)
		{
			return TextureHandle();
		}

		// Add texture to pool
		TextureHandle new_texture_handle = m_texture_pool.add();

		Texture* new_texture = m_texture_pool.get(new_texture_handle);
		new_texture->info = texture_info;
		new_texture->d3d_texture_resource = texture_resource;

		return new_texture_handle;
	}

	TextureHandle TextureSystem::create_texture(const TextureInfo& texture_info, const void* init_data)
	{
		TextureHandle new_texture_handle;

		GraphicsAPI::Device* device = m_graphics_api.get_device();
		D3DResource new_d3d_texture;

		if (texture_info.dimensions.z > 0)
		{
			// 3D texture
			// TODO!!!
		}
		else if (texture_info.dimensions.y > 0)
		{
			// 2D texture
			const D3D11_TEXTURE2D_DESC texture_description = create_2d_texture_desc(texture_info);

			D3D11_SUBRESOURCE_DATA subresource_data;
			ZeroMemory(&subresource_data, sizeof(D3D11_SUBRESOURCE_DATA));

			if (init_data)
			{
				subresource_data.pSysMem = init_data;
				subresource_data.SysMemPitch = texture_description.Width * static_cast<UINT>(get_bits_per_pixel(texture_description.Format)) / 8; // FIXME: use DDS loader utilities!
				subresource_data.SysMemSlicePitch = 0; // TODO: when to set this?
			}

			// TODO: when mipmaps and texture arrays get involved, we need to make a subresource for those!

			D3DTexture2D d3d_texture_2d;
			const HRESULT result = device->CreateTexture2D(&texture_description, init_data ? &subresource_data : nullptr, d3d_texture_2d.ReleaseAndGetAddressOf());
			if (FAILED(result))
			{
				// TODO: error message!
				return new_texture_handle;
			}

			new_d3d_texture = d3d_texture_2d;
		}
		else
		{
			// 1D texture
			// TODO!!!
		}

		// Add texture to pool
		new_texture_handle = m_texture_pool.add();

		Texture* new_texture = m_texture_pool.get(new_texture_handle);
		new_texture->info = texture_info;
		new_texture->d3d_texture_resource = new_d3d_texture;

		return new_texture_handle;
	}

	void TextureSystem::remove_texture(TextureHandle texture_handle)
	{
		Texture* texture = m_texture_pool.get(texture_handle);
		if (!texture)
		{
			return;
		}

		// NOTE: we do not remove the SRV, only the texture (client is responsible for full cleanup)
		texture->d3d_texture_resource.Reset();
		m_texture_pool.remove(texture_handle);
	}

	TextureInfo TextureSystem::get_texture_info(TextureHandle texture_handle) const
	{
		const Texture* texture = m_texture_pool.get(texture_handle);
		if (texture == nullptr)
		{
			return TextureInfo();
		}

		return texture->info;
	}

	ResourceViewHandle TextureSystem::create_resource_view(TextureHandle texture_handle, const ResourceViewInfo& srv_info)
	{
		Texture* texture = m_texture_pool.get(texture_handle);
		if (texture == nullptr)
		{
			return ResourceViewHandle();
		}

		// Create shader resource view from D3D resource
		ShaderSystem& shader_system = m_graphics_api.get_directx_shader_system();
		return shader_system.create_resource_view(texture->d3d_texture_resource.Get(), srv_info);
	}

	TextureSamplerHandle TextureSystem::create_sampler(const TextureSamplerInfo& sampler_info)
	{
		TextureSamplerHandle new_sampler_handle;

		D3D11_SAMPLER_DESC sampler_description;
		ZeroMemory(&sampler_description, sizeof(D3D11_SAMPLER_DESC));

		sampler_description.Filter = get_d3d_filter(sampler_info.filter);
		sampler_description.AddressU = get_d3d_texture_address_mode(sampler_info.address_u);
		sampler_description.AddressV = get_d3d_texture_address_mode(sampler_info.address_v);
		sampler_description.AddressW = get_d3d_texture_address_mode(sampler_info.address_w);
		sampler_description.MipLODBias = sampler_info.mip_lod_bias;
		sampler_description.ComparisonFunc = get_d3d_comparison_func(sampler_info.comparison_func);
		sampler_description.MinLOD = sampler_info.min_lod;
		sampler_description.MaxLOD = sampler_info.max_lod;

		D3DSamplerState new_sampler_state;
		GraphicsAPI::Device* device = m_graphics_api.get_device();

		const HRESULT result = device->CreateSamplerState(&sampler_description, new_sampler_state.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			return new_sampler_handle;
		}

		new_sampler_handle = m_sampler_pool.add();

		TextureSampler* new_sampler = m_sampler_pool.get(new_sampler_handle);
		new_sampler->info = sampler_info;
		new_sampler->sampler_state = new_sampler_state;

		return new_sampler_handle;
	}

	void TextureSystem::remove_sampler(TextureSamplerHandle sampler_handle)
	{
		TextureSampler* sampler = m_sampler_pool.get(sampler_handle);
		if (!sampler)
		{
			return;
		}

		sampler->sampler_state.Reset();

		m_sampler_pool.remove(sampler_handle);
	}

	void TextureSystem::set_sampler(ShaderType shader_type, TextureSamplerHandle sampler_handle, int slot)
	{
		TextureSampler* sampler = m_sampler_pool.get(sampler_handle);
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		ID3D11SamplerState* sampler_array[] = { sampler ? sampler->sampler_state.Get() : nullptr };

		switch (shader_type)
		{
		case ShaderType::VERTEX:
			device_context->VSSetSamplers(slot, 1, sampler_array);
			break;
		case ShaderType::PIXEL:
			device_context->PSSetSamplers(slot, 1, sampler_array);
			break;
		}
	}

	TextureSystem::TextureSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::TextureSystem(core)
		, m_graphics_api(graphics_api)
	{
	}

	bool TextureSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void TextureSystem::shutdown()
	{
		log("Shutting down Texture system (DirectX).\n");

		// Clear pools
		// TODO: add warning in case of leftover resources?
		m_texture_pool.reset();
		m_sampler_pool.reset();

		log("Texture system (DirectX) shut down successfully.\n");
	}
}