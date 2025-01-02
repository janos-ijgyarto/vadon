#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/ResourceSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/Buffer.hpp>

namespace
{
	constexpr UINT get_d3d_copy_flags(Vadon::Render::CopyFlags flags)
	{
		UINT d3d_flags = 0;
		if (flags == Vadon::Render::CopyFlags::NONE)
		{
			return d3d_flags;
		}

		if (Vadon::Utilities::to_integral(flags) & Vadon::Utilities::to_integral(Vadon::Render::CopyFlags::NO_OVERWRITE))
		{
			d3d_flags |= D3D11_COPY_NO_OVERWRITE;
		}

		if (Vadon::Utilities::to_integral(flags) & Vadon::Utilities::to_integral(Vadon::Render::CopyFlags::DISCARD))
		{
			d3d_flags |= D3D11_COPY_DISCARD;
		}

		return d3d_flags;
	}

	constexpr D3D11_MAP get_d3d_map(Vadon::Render::MapType map_type)
	{
		switch(map_type)
		{
		case Vadon::Render::MapType::READ:
			return D3D11_MAP_READ;
		case Vadon::Render::MapType::WRITE:
			return D3D11_MAP_WRITE;
		case Vadon::Render::MapType::READ_WRITE:
			return D3D11_MAP_READ_WRITE;
		case Vadon::Render::MapType::WRITE_DISCARD:
			return D3D11_MAP_WRITE_DISCARD;
		case Vadon::Render::MapType::WRITE_NO_OVERWRITE:
			return D3D11_MAP_WRITE_NO_OVERWRITE;
		};

		return D3D11_MAP_READ;
	}

	constexpr UINT get_d3d_map_flags(Vadon::Render::MapFlags map_flags)
	{
		UINT d3d_flags = 0;
		if (map_flags == Vadon::Render::MapFlags::NONE)
		{
			return d3d_flags;
		}

		if (Vadon::Utilities::to_integral(map_flags) & Vadon::Utilities::to_integral(Vadon::Render::MapFlags::DO_NOT_WAIT))
		{
			d3d_flags |= D3D11_MAP_FLAG_DO_NOT_WAIT;
		}

		return d3d_flags;
	}

	constexpr UINT get_d3d_buffer_uav_flags(Vadon::Render::BufferUAVFlags flags)
	{
		UINT d3d_flags = 0;
		if (flags == Vadon::Render::BufferUAVFlags::NONE)
		{
			return d3d_flags;
		}

		if (Vadon::Utilities::to_bool(flags & Vadon::Render::BufferUAVFlags::RAW))
		{
			d3d_flags |= D3D11_BUFFER_UAV_FLAG_RAW;
		}

		if (Vadon::Utilities::to_bool(flags & Vadon::Render::BufferUAVFlags::APPEND))
		{
			d3d_flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
		}

		if (Vadon::Utilities::to_bool(flags & Vadon::Render::BufferUAVFlags::COUNTER))
		{
			d3d_flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
		}

		return d3d_flags;
	}

	constexpr D3D11_SRV_DIMENSION get_d3d_srv_dimension(Vadon::Render::ShaderResourceViewType srv_type)
	{
		switch (srv_type)
		{
		case Vadon::Render::ShaderResourceViewType::UNKNOWN:
			return D3D11_SRV_DIMENSION_UNKNOWN;
		case Vadon::Render::ShaderResourceViewType::BUFFER:
			return D3D11_SRV_DIMENSION_BUFFER;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_1D:
			return D3D11_SRV_DIMENSION_TEXTURE1D;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_1D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_2D:
			return D3D11_SRV_DIMENSION_TEXTURE2D;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_2D_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_2D_MS:
			return D3D11_SRV_DIMENSION_TEXTURE2DMS;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_2D_MS_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_3D:
			return D3D11_SRV_DIMENSION_TEXTURE3D;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_CUBE:
			return D3D11_SRV_DIMENSION_TEXTURECUBE;
		case Vadon::Render::ShaderResourceViewType::TEXTURE_CUBE_ARRAY:
			return D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		case Vadon::Render::ShaderResourceViewType::RAW_BUFFER:
			return D3D11_SRV_DIMENSION_BUFFEREX;
		}

		return D3D11_SRV_DIMENSION_UNKNOWN;
	}

	D3D11_UAV_DIMENSION get_d3d_uav_dimension(Vadon::Render::UnorderedAccessViewType uav_type)
	{
		switch (uav_type)
		{
		case Vadon::Render::UnorderedAccessViewType::UNKNOWN:
			return D3D11_UAV_DIMENSION_UNKNOWN;
		case Vadon::Render::UnorderedAccessViewType::BUFFER:
			return D3D11_UAV_DIMENSION_BUFFER;
		case Vadon::Render::UnorderedAccessViewType::TEXTURE_1D:
			return D3D11_UAV_DIMENSION_TEXTURE1D;
		case Vadon::Render::UnorderedAccessViewType::TEXTURE_1D_ARRAY:
			return D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		case Vadon::Render::UnorderedAccessViewType::TEXTURE_2D:
			return D3D11_UAV_DIMENSION_TEXTURE2D;
		case Vadon::Render::UnorderedAccessViewType::TEXTURE_2D_ARRAY:
			return D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		case Vadon::Render::UnorderedAccessViewType::TEXTURE_3D:
			return D3D11_UAV_DIMENSION_TEXTURE3D;
		}

		return D3D11_UAV_DIMENSION_UNKNOWN;
	}

	D3D11_BOX get_d3d_box(const Vadon::Render::ResourceBox& box)
	{
		return D3D11_BOX{
			box.left,
			box.top,
			box.front,
			box.right,
			box.bottom,
			box.back
		};
	}

	void apply_d3d_shader_resources(Vadon::Render::ShaderType shader_type, Vadon::Private::Render::DirectX::GraphicsAPI::DeviceContext* device_context, UINT start_slot, UINT num_views, ID3D11ShaderResourceView* const* shader_resource_views)
	{
		switch (shader_type)
		{
		case Vadon::Render::ShaderType::VERTEX:
			device_context->VSSetShaderResources(start_slot, num_views, shader_resource_views);
			break;
		case Vadon::Render::ShaderType::GEOMETRY:
			device_context->GSSetShaderResources(start_slot, num_views, shader_resource_views);
			break;
		case Vadon::Render::ShaderType::PIXEL:
			device_context->PSSetShaderResources(start_slot, num_views, shader_resource_views);
			break;
		case Vadon::Render::ShaderType::COMPUTE:
			device_context->CSSetShaderResources(start_slot, num_views, shader_resource_views);
			break;
		}
	}
}

namespace Vadon::Private::Render::DirectX
{
	const Resource* ResourceSystem::get_base_resource(Vadon::Render::ResourceHandle resource_handle) const
	{
		const ResourceWrapper& resource_wrapper = m_resource_pool.get(resource_handle);
		return resource_wrapper.resource;
	}

	void ResourceSystem::remove_resource(ResourceHandle resource_handle)
	{
		ResourceWrapper& resource_wrapper = m_resource_pool.get(resource_handle);
		delete resource_wrapper.resource;
		resource_wrapper.resource = nullptr;

		m_resource_pool.remove(resource_handle);
	}

	SRVHandle ResourceSystem::create_srv(ResourceHandle resource_handle, const ShaderResourceViewInfo& srv_info)
	{
		// TODO: check existing SRVs for resource and see if we already have a compatible one?
		D3D11_SHADER_RESOURCE_VIEW_DESC d3d_srv_desc;
		ZeroMemory(&d3d_srv_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		d3d_srv_desc.Format = get_dxgi_format(srv_info.format);
		d3d_srv_desc.ViewDimension = get_d3d_srv_dimension(srv_info.type);

		// FIXME: implement other types!
		switch (srv_info.type)
		{
		case ShaderResourceViewType::BUFFER:
		{
			d3d_srv_desc.Buffer.FirstElement = srv_info.type_info.first_array_slice;
			d3d_srv_desc.Buffer.NumElements = srv_info.type_info.array_size;
		}
		break;
		case ShaderResourceViewType::TEXTURE_2D:
		{
			d3d_srv_desc.Texture2D.MipLevels = srv_info.type_info.mip_levels;
			d3d_srv_desc.Texture2D.MostDetailedMip = srv_info.type_info.most_detailed_mip;
		}
		break;
		default:
			log_error("Graphics resource system: SRV type not supported!\n");
			return SRVHandle();
		}

		D3DShaderResourceView d3d_srv;

		Resource* resource = get_resource<Resource>(D3DResourceHandle::from_resource_handle(resource_handle));

		// TODO: allow passing nullptr to SRV desc to create a "default view"?
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		const HRESULT result = device->CreateShaderResourceView(resource->d3d_resource.Get(), &d3d_srv_desc, d3d_srv.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			log_error("Graphics resource system: failed to create SRV!\n");
			return SRVHandle();
		}

		// Success, add SRV to pool
		SRVHandle new_srv_handle = m_srv_pool.add();

		ShaderResourceView& new_srv = m_srv_pool.get(new_srv_handle);
		new_srv.info = srv_info;
		new_srv.d3d_srv = d3d_srv;
		new_srv.resource = resource_handle;

		return new_srv_handle;
	}

	ResourceHandle ResourceSystem::get_srv_resource(SRVHandle srv_handle) const
	{
		const ShaderResourceView& srv = m_srv_pool.get(srv_handle);
		return srv.resource;
	}

	void ResourceSystem::remove_srv(SRVHandle srv_handle)
	{
		ShaderResourceView& srv = m_srv_pool.get(srv_handle);
		srv.d3d_srv.Reset();

		m_srv_pool.remove(srv_handle);
	}

	UAVHandle ResourceSystem::create_uav(ResourceHandle resource_handle, const UnorderedAccessViewInfo& uav_info)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC d3d_uav_desc;
		ZeroMemory(&d3d_uav_desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

		d3d_uav_desc.Format = get_dxgi_format(uav_info.format);
		d3d_uav_desc.ViewDimension = get_d3d_uav_dimension(uav_info.type);

		// FIXME: implement other types!
		switch (uav_info.type)
		{
		case UnorderedAccessViewType::BUFFER:
		{
			d3d_uav_desc.Buffer.FirstElement = uav_info.type_info.mip_slice;
			d3d_uav_desc.Buffer.NumElements = uav_info.type_info.first_array_slice;
			d3d_uav_desc.Buffer.Flags = uav_info.type_info.array_size;
		}
		break;
		case UnorderedAccessViewType::TEXTURE_2D:
		{
			d3d_uav_desc.Texture2D.MipSlice = uav_info.type_info.mip_slice;
		}
		break;
		default:
			log_error("Graphics resource system: UAV type not supported!\n");
			return UAVHandle();
		}

		D3DUnorderedAccessView d3d_uav;

		Resource* resource = get_resource<Resource>(D3DResourceHandle::from_resource_handle(resource_handle));

		// TODO: allow passing nullptr to SRV desc to create a "default view"?
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		const HRESULT result = device->CreateUnorderedAccessView(resource->d3d_resource.Get(), &d3d_uav_desc, d3d_uav.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			log_error("Graphics resource system: failed to create UAV!\n");
			return UAVHandle();
		}

		// Success, add UAV to pool
		UAVHandle new_uav_handle = m_uav_pool.add();

		UnorderedAccessView& new_uav = m_uav_pool.get(new_uav_handle);
		new_uav.info = uav_info;
		new_uav.d3d_uav = d3d_uav;
		new_uav.resource = resource_handle;

		return new_uav_handle;
	}

	ResourceHandle ResourceSystem::get_uav_resource(UAVHandle uav_handle) const
	{
		const UnorderedAccessView& uav = m_uav_pool.get(uav_handle);
		return uav.resource;
	}

	void ResourceSystem::remove_uav(UAVHandle uav_handle)
	{
		UnorderedAccessView& uav = m_uav_pool.get(uav_handle);
		uav.d3d_uav.Reset();
		
		m_uav_pool.remove(uav_handle);
	}

	void ResourceSystem::update_subresource(ResourceHandle resource_handle, uint32_t subresource, const ResourceBox* box, const void* source_data, uint32_t source_row_pitch, uint32_t source_depth_pitch, CopyFlags flags)
	{
		Resource* resource = get_resource<Resource>(D3DResourceHandle::from_resource_handle(resource_handle));

		D3D11_BOX d3d_box;
		if (box != nullptr)
		{
			d3d_box = get_d3d_box(*box);
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->UpdateSubresource1(resource->d3d_resource.Get(), subresource, box ? &d3d_box : NULL, source_data, source_row_pitch, source_depth_pitch, get_d3d_copy_flags(flags));
	}

	bool ResourceSystem::map_resource(ResourceHandle resource_handle, uint32_t subresource, MapType map_type, MappedSubresource* mapped_subresource, MapFlags flags)
	{
		Resource* resource = get_resource<Resource>(D3DResourceHandle::from_resource_handle(resource_handle));

		D3D11_MAPPED_SUBRESOURCE d3d_mapped_subresource;
		ZeroMemory(&d3d_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		HRESULT hr = device_context->Map(resource->d3d_resource.Get(), subresource, get_d3d_map(map_type), get_d3d_map_flags(flags), mapped_subresource ? &d3d_mapped_subresource : NULL);
		if (FAILED(hr))
		{
			log_error("Graphics resource system: failed to map resource!\n");
			return false;
		}

		if (mapped_subresource != nullptr)
		{
			mapped_subresource->data = d3d_mapped_subresource.pData;
			mapped_subresource->row_pitch = d3d_mapped_subresource.RowPitch;
			mapped_subresource->depth_pitch = d3d_mapped_subresource.DepthPitch;
		}

		return true;
	}

	void ResourceSystem::unmap_resource(ResourceHandle resource_handle, uint32_t subresource)
	{
		Resource* resource = get_resource<Resource>(D3DResourceHandle::from_resource_handle(resource_handle));

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->Unmap(resource->d3d_resource.Get(), subresource);
	}

	void ResourceSystem::apply_shader_resource(ShaderType shader_type, SRVHandle srv_handle, uint32_t slot)
	{
		ID3D11ShaderResourceView* resource_array[] = { nullptr };
		if (srv_handle.is_valid() == true)
		{
			const ShaderResourceView& srv = m_srv_pool.get(srv_handle);
			resource_array[0] = srv.d3d_srv.Get();
		}

		apply_d3d_shader_resources(shader_type, m_graphics_api.get_device_context(), slot, 1, resource_array);
	}

	void ResourceSystem::apply_shader_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views)
	{
		// TODO: use member vector!
		static std::vector<ID3D11ShaderResourceView*> temp_srv_vector;
		temp_srv_vector.clear();
		temp_srv_vector.reserve(resource_views.resources.size());

		for (const SRVHandle& current_srv_handle : resource_views.resources)
		{
			ID3D11ShaderResourceView* current_d3d_srv = nullptr;
			if (current_srv_handle.is_valid() == true)
			{
				const ShaderResourceView& current_srv = m_srv_pool.get(current_srv_handle);
				current_d3d_srv = current_srv.d3d_srv.Get();
			}
			temp_srv_vector.push_back(current_d3d_srv);
		}

		apply_d3d_shader_resources(shader_type, m_graphics_api.get_device_context(), resource_views.start_slot, static_cast<UINT>(resource_views.resources.size()), temp_srv_vector.data());
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::ResourceSystem(core)
		, m_graphics_api(graphics_api)
	{

	}

	bool ResourceSystem::initialize()
	{
		log_message("Initializing Resource system (DirectX).\n");
		// TODO: anything?
		log_message("Resource system (DirectX) initialized successfully.\n");
		return true;
	}

	void ResourceSystem::shutdown()
	{
		log_message("Shutting down Resource system (DirectX).\n");
		for (auto resource_pair : m_resource_pool)
		{
			ResourceWrapper* current_resource_wrapper = resource_pair.second;
			if (current_resource_wrapper->resource != nullptr)
			{
				delete current_resource_wrapper->resource;
				current_resource_wrapper->resource = nullptr;
			}
		}
		m_resource_pool.reset();
		m_srv_pool.reset();
		m_uav_pool.reset();
		log_message("Resource system (DirectX) shut down successfully.\n");
	}

	ResourceHandle ResourceSystem::internal_create_resource(ResourceType type, D3DResource d3d_resource, Resource* resource)
	{
		// FIXME: make sure to validate that type and D3D resource type match?
		resource->d3d_resource = d3d_resource;
		resource->type = type;

		ResourceHandle new_resource_handle = m_resource_pool.add();
		
		ResourceWrapper& new_resource_wrapper = m_resource_pool.get(new_resource_handle);
		new_resource_wrapper.resource = resource;
		
		return new_resource_handle;
	}
}