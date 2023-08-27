#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/BufferSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>

#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Private::Render::DirectX
{
	D3D11_USAGE get_d3d_usage(BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::DEFAULT:
			return D3D11_USAGE::D3D11_USAGE_DEFAULT;
		case BufferUsage::IMMUTABLE:
			return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
		case BufferUsage::DYNAMIC:
			return D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		case BufferUsage::STAGING:
			return D3D11_USAGE::D3D11_USAGE_STAGING;
		}

		return D3D11_USAGE::D3D11_USAGE_DEFAULT;
	}

	BufferUsage get_buffer_usage(D3D11_USAGE usage)
	{
		switch (usage)
		{
		case D3D11_USAGE::D3D11_USAGE_DEFAULT:
			return BufferUsage::DEFAULT;
		case D3D11_USAGE::D3D11_USAGE_IMMUTABLE:
			return BufferUsage::IMMUTABLE;
		case D3D11_USAGE::D3D11_USAGE_DYNAMIC:
			return BufferUsage::DYNAMIC;
		case D3D11_USAGE::D3D11_USAGE_STAGING:
			return BufferUsage::STAGING;
		}

		return BufferUsage::DEFAULT;
	}

	D3D11_BIND_FLAG get_d3d_bind_flags(BufferBindFlags bind_flags)
	{
		UINT d3d_bind_flags = 0;
		VADON_START_BITMASK_SWITCH(bind_flags)
		{
		case BufferBindFlags::VERTEX:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferBindFlags::INDEX:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferBindFlags::CONSTANT:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
			break;
		case BufferBindFlags::SHADER_RESOURCE:
			d3d_bind_flags |= D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
			break;
		}

		return D3D11_BIND_FLAG(d3d_bind_flags);
	}

	BufferBindFlags get_buffer_bind_flags(D3D11_BIND_FLAG d3d_bind_flags)
	{
		BufferBindFlags bind_flags = BufferBindFlags::NONE;

		VADON_START_BITMASK_SWITCH(d3d_bind_flags)
		{
		case D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER:
			bind_flags |= BufferBindFlags::VERTEX;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER:
			bind_flags |= BufferBindFlags::INDEX;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER:
			bind_flags |= BufferBindFlags::CONSTANT;
			break;
		case D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE:
			bind_flags |= BufferBindFlags::SHADER_RESOURCE;
			break;
		}

		return bind_flags;
	}

	BufferHandle BufferSystem::create_buffer(const BufferInfo& buffer_info, const void* init_data)
	{
		D3D11_BUFFER_DESC buffer_description;
		ZeroMemory(&buffer_description, sizeof(D3D11_BUFFER_DESC));

		buffer_description.ByteWidth = buffer_info.capacity * buffer_info.element_size;
		buffer_description.Usage = get_d3d_usage(buffer_info.usage); // TODO: some buffers cannot trivially assign the same usage (e.g read-write)
		buffer_description.BindFlags = get_d3d_bind_flags(buffer_info.bind_flags);

		if (buffer_info.usage == BufferUsage::DYNAMIC)
		{
			// Dynamic buffers need write access
			buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		buffer_description.MiscFlags = 0; // TODO: add support for flags, e.g structured buffers

		GraphicsAPI::Device* device = m_graphics_api.get_device();
		ComPtr<ID3D11Buffer> new_d3d_buffer;

		D3D11_SUBRESOURCE_DATA subresource_data;
		ZeroMemory(&subresource_data, sizeof(D3D11_SUBRESOURCE_DATA));

		if (init_data)
		{
			subresource_data.pSysMem = init_data;
		}

		HRESULT hr = device->CreateBuffer(&buffer_description, init_data ? &subresource_data : nullptr, new_d3d_buffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			// TODO: error handling?
			return BufferHandle();
		}

		// TODO: handle additional setup, e.g SRVs and UAVs

		const BufferHandle new_buffer_handle = m_buffer_pool.add(buffer_info);

		Buffer* new_buffer = m_buffer_pool.get(new_buffer_handle);
		new_buffer->d3d_buffer = new_d3d_buffer;

		return new_buffer_handle;
	}

	void BufferSystem::remove_buffer(BufferHandle buffer_handle)
	{
		Buffer* buffer = m_buffer_pool.get(buffer_handle);
		if (!buffer)
		{
			return;
		}

		// Clean up the D3D resources
		buffer->d3d_buffer.Reset();

		m_buffer_pool.remove(buffer_handle);
	}

	bool BufferSystem::buffer_data(BufferHandle buffer_handle, const Vadon::Utilities::DataRange& range, const void* data, bool force_discard)
	{
		const Buffer* buffer = m_buffer_pool.get(buffer_handle);
		if (!buffer)
		{
			return false;
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		switch (buffer->info.usage)
		{
		case BufferUsage::DEFAULT:
		{
			// FIXME: revise box usage and source row/depth pitch
			const bool is_not_cbuffer = Utilities::to_bool(buffer->info.bind_flags & BufferBindFlags::CONSTANT);
			const uint32_t source_row_pitch = is_not_cbuffer ? (buffer->info.element_size * range.count) : (buffer->info.element_size * buffer->info.capacity);

			// Create a box that defines the region we want to update (need to use byte widths for the dimensions)
			D3D11_BOX dest_box{
				static_cast<UINT>(buffer->info.element_size * range.offset), // Left
				0, // Top
				0, // Front
				static_cast<UINT>(buffer->info.element_size * (range.offset + range.count)), // Right
				1, // Bottom
				1 // Back
			};

			device_context->UpdateSubresource(buffer->d3d_buffer.Get(), 0, is_not_cbuffer ? &dest_box : nullptr, data, source_row_pitch, 0);
			return true;
		}
		case BufferUsage::IMMUTABLE:
			// TODO: what goes here?
			break;
		case BufferUsage::DYNAMIC:
		case BufferUsage::STAGING:
		{
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			ZeroMemory(&mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			// Do not allow "NO_OVERWRITE" if it's a shader resource
			// FIXME: allow if we are on D3D 11.1
			const D3D11_MAP map_type = (Utilities::to_bool(buffer->info.bind_flags & (BufferBindFlags::VERTEX | BufferBindFlags::INDEX)) && (force_discard == false))
				? D3D11_MAP::D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP::D3D11_MAP_WRITE_DISCARD;

			// TODO: other parameters?
			const HRESULT result = device_context->Map(buffer->d3d_buffer.Get(), 0, map_type, 0, &mapped_subresource);
			if (FAILED(result))
			{
				// TODO: error handling?
				return false;
			}

			void* offset_pData = static_cast<char*>(mapped_subresource.pData) + (range.offset * buffer->info.element_size);
			memcpy(offset_pData, data, buffer->info.element_size * range.count);

			device_context->Unmap(buffer->d3d_buffer.Get(), 0);
			return true;
		}
		}

		return false;
	}

	void BufferSystem::set_vertex_buffer(BufferHandle buffer_handle, int slot)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		const Buffer* buffer = m_buffer_pool.get(buffer_handle);
		if (buffer == nullptr)
		{
			// Unset the buffer in the slot
			device_context->IASetVertexBuffers(slot, 1, nullptr, nullptr, nullptr);
			return;
		}

		if (Utilities::to_bool(buffer->info.bind_flags & BufferBindFlags::VERTEX) == false)
		{
			// TODO: error message?
			return;
		}

		// FIXME: this should be configurable!
		ID3D11Buffer* vertex_buffers[] = { buffer->d3d_buffer.Get() };
		UINT element_sizes[] = { static_cast<UINT>(buffer->info.element_size) };
		constexpr UINT NULL_OFFSET = 0;

		// TODO: keep track of what buffers are set where!
		device_context->IASetVertexBuffers(slot, 1, vertex_buffers, element_sizes, &NULL_OFFSET);
	}

	void BufferSystem::set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		const Buffer* buffer = m_buffer_pool.get(buffer_handle);
		if (buffer == nullptr)
		{
			// Unset the index buffer
			device_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			return;
		}

		if (Utilities::to_bool(buffer->info.bind_flags & BufferBindFlags::INDEX) == false)
		{
			// TODO: error message?
			return;
		}

		// TODO: keep track of what buffers are set where!
		device_context->IASetIndexBuffer(buffer->d3d_buffer.Get(), get_dxgi_format(format), 0);
	}

	void BufferSystem::set_constant_buffer(BufferHandle buffer_handle, int slot)
	{
		const Buffer* buffer = m_buffer_pool.get(buffer_handle);
		if (!buffer)
		{
			// TODO: error message?
			return;
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		ID3D11Buffer* constant_buffers[] = { buffer->d3d_buffer.Get() };

		// Set the buffer in our shaders (unset if allocation is invalid)
		device_context->VSSetConstantBuffers(slot, 1, constant_buffers);
		device_context->GSSetConstantBuffers(slot, 1, constant_buffers);
		device_context->PSSetConstantBuffers(slot, 1, constant_buffers);
	}

	BufferSystem::BufferSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::BufferSystem(core)
		, m_graphics_api(graphics_api)
	{
	}

	bool BufferSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void BufferSystem::shutdown()
	{
		log("Shutting down Buffer system (DirectX).\n");

		// Clear buffers
		// TODO: add warning in case of leftover buffers?
		m_buffer_pool.reset();

		log("Buffer system (DirectX) shut down successfully.\n");
	}
}