#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/BufferSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Private::Render::DirectX
{
	namespace
	{
		UINT get_buffer_bind_flags(const BufferInfo& buffer_info)
		{
			UINT bind_flags = 0;

			VADON_START_BITMASK_SWITCH(buffer_info.flags)
			{
			case BufferFlags::SHADER_RESOURCE:
				bind_flags |= D3D11_BIND_SHADER_RESOURCE;
				break;
			case BufferFlags::UNORDERED_ACCESS:
				bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
				break;
			case BufferFlags::RENDER_TARGET:
				bind_flags |= D3D11_BIND_RENDER_TARGET;
				break;
			}
			return bind_flags;
		}

		D3D11_BUFFER_DESC create_buffer_description(const BufferInfo& buffer_info)
		{
			D3D11_BUFFER_DESC buffer_description;
			ZeroMemory(&buffer_description, sizeof(D3D11_BUFFER_DESC));

			buffer_description.ByteWidth = buffer_info.capacity * buffer_info.element_size;
			buffer_description.Usage = get_d3d_usage(buffer_info.usage);

			// TODO: do we pre-filter these based on buffer type?
			buffer_description.BindFlags = get_buffer_bind_flags(buffer_info);
			buffer_description.CPUAccessFlags = get_d3d_cpu_access_flags(buffer_info.access_flags);

			switch (buffer_info.type)
			{
			case BufferType::VERTEX:
			{
				buffer_description.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
			}
			break;
			case BufferType::INDEX:
			{
				buffer_description.BindFlags |= D3D11_BIND_INDEX_BUFFER;
			}
			break;
			case BufferType::CONSTANT:
			{
				// FIXME: send error if any other bind flags were set?
				buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			}
			break;
			case BufferType::RAW:
			{
				// FIXME: we are mandating SRV for this buffer type, should we allow user to opt out?
				buffer_description.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				buffer_description.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			}
			break;
			case BufferType::TYPED:
			{
				// FIXME: we are mandating SRV for this buffer type, should we allow user to opt out?
				buffer_description.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			}
			break;
			case BufferType::STRUCTURED:
			{
				// FIXME: we are mandating SRV for this buffer type, should we allow user to opt out?
				buffer_description.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				buffer_description.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
				buffer_description.StructureByteStride = buffer_info.element_size;
			}
			break;
			}

			return buffer_description;
		}

		void set_constant_buffers(GraphicsAPI::DeviceContext* device_context, ShaderType shader, UINT start_slot, UINT num_buffers, ID3D11Buffer* const* constant_buffers)
		{
			// Set the buffer in the shader
			switch (shader)
			{
			case ShaderType::VERTEX:
				device_context->VSSetConstantBuffers(start_slot, num_buffers, constant_buffers);
				break;
			case ShaderType::GEOMETRY:
				device_context->GSSetConstantBuffers(start_slot, num_buffers, constant_buffers);
				break;
			case ShaderType::PIXEL:
				device_context->PSSetConstantBuffers(start_slot, num_buffers, constant_buffers);
				break;
			case ShaderType::COMPUTE:
				device_context->CSSetConstantBuffers(start_slot, num_buffers, constant_buffers);
				break;
			}
		}

		constexpr ShaderResourceViewType get_srv_type(BufferSRVType srv_type)
		{
			switch (srv_type)
			{
			case BufferSRVType::BUFFER:
				return ShaderResourceViewType::BUFFER;
			case BufferSRVType::RAW_BUFFER:
				return ShaderResourceViewType::RAW_BUFFER;
			}

			return ShaderResourceViewType::UNKNOWN;
		}

		std::vector<ID3D11Buffer*> temp_buffer_vector;
	}

	BufferHandle BufferSystem::create_buffer(const BufferInfo& buffer_info, const void* init_data)
	{
		D3D11_BUFFER_DESC buffer_description = create_buffer_description(buffer_info);

		if (buffer_info.usage == ResourceUsage::DYNAMIC)
		{
			// Set write flag
			// FIXME: should we warn user if this was not done correctly?
			buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

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
			log_error("Buffer system: failed to create D3D buffer!\n");
			return BufferHandle();
		}

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();

		const D3DBufferHandle new_buffer_handle = dx_resource_system.create_resource<Buffer>(ResourceType::BUFFER, new_d3d_buffer);

		Buffer* new_buffer = dx_resource_system.get_resource<Buffer>(new_buffer_handle);
		new_buffer->info = buffer_info;
		new_buffer->d3d_buffer = new_d3d_buffer;

		return BufferHandle::from_resource_handle(new_buffer_handle);
	}

	bool BufferSystem::is_buffer_valid(BufferHandle buffer_handle) const
	{
		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		return dx_resource_system.is_resource_valid(buffer_handle);
	}

	void BufferSystem::remove_buffer(BufferHandle buffer_handle)
	{
		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		dx_resource_system.remove_resource(buffer_handle);
	}

	SRVHandle BufferSystem::create_buffer_srv(BufferHandle buffer_handle, const BufferSRVInfo& buffer_srv_info)
	{
		ShaderResourceViewInfo srv_info;
		srv_info.type = get_srv_type(buffer_srv_info.type);
		srv_info.format = buffer_srv_info.format;

		// TODO: raw buffer flags!
		srv_info.type_info.first_array_slice = buffer_srv_info.first_element;
		srv_info.type_info.array_size = buffer_srv_info.num_elements;

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		return dx_resource_system.create_srv(buffer_handle, srv_info);
	}

	bool BufferSystem::buffer_data(BufferHandle buffer_handle, const BufferWriteData& write_data)
	{
		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		const Buffer* buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(buffer_handle));
		const BufferInfo& buffer_info = buffer->info;

		// FIXME: revise API slightly. Cannot use UpdateSubResource on DYNAMIC, but apparently can on STAGING, and Map can be used on DEFAULT?
		switch (buffer_info.usage)
		{
		case ResourceUsage::DEFAULT:
		{
			// FIXME: cbuffer can contain array, should we allow updating a portion of it?
			const bool is_not_cbuffer = buffer_info.type != BufferType::CONSTANT;
			const uint32_t source_row_pitch = is_not_cbuffer ? (buffer_info.element_size * write_data.range.count) : (buffer_info.element_size * buffer_info.capacity);

			// Create a box that defines the region we want to update (need to use byte widths for the dimensions)
			// Buffers are all flat, so we only need the L-R portions
			ResourceBox dest_box{
				.left = static_cast<UINT>(buffer_info.element_size * write_data.range.offset),
				.top = 0,
				.front = 0,
				.right = static_cast<UINT>(buffer_info.element_size * (write_data.range.offset + write_data.range.count)),
				.bottom = 1,
				.back = 1
			}; 

			// FIXME: subresource index?
			dx_resource_system.update_subresource(buffer_handle, 0, &dest_box, write_data.data, source_row_pitch, 0, (write_data.no_overwrite == true) ? CopyFlags::NO_OVERWRITE : CopyFlags::DISCARD);
			return true;
		}
		case ResourceUsage::IMMUTABLE:
			return false;
		case ResourceUsage::DYNAMIC:
		case ResourceUsage::STAGING:
		{			
			// Do not allow "NO_OVERWRITE" for cbuffer or shader resource
			// FIXME: allow if we are on D3D 11.1
			const bool can_use_no_overwrite = (buffer_info.type == BufferType::VERTEX) || (buffer_info.type == BufferType::INDEX);
			const MapType map_type = (can_use_no_overwrite && (write_data.no_overwrite == true)) ? MapType::WRITE_NO_OVERWRITE : MapType::WRITE_DISCARD;

			MappedSubresource mapped_subresource;

			// TODO: enable map flags?
			if (dx_resource_system.map_resource(buffer_handle, 0, map_type, &mapped_subresource, MapFlags::NONE) == false)
			{
				log_error("Buffer system: failed to map D3D buffer!\n");
				return false;
			}

			void* offset_data = static_cast<char*>(mapped_subresource.data) + (write_data.range.offset * buffer_info.element_size);
			memcpy(offset_data, write_data.data, buffer_info.element_size * write_data.range.count);

			dx_resource_system.unmap_resource(buffer_handle, 0);
			return true;
		}
		}

		return false;
	}

	void BufferSystem::set_vertex_buffer(BufferHandle buffer_handle, int32_t slot, int32_t stride, int32_t offset)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		if (buffer_handle.is_valid() == false)
		{
			// Unset the buffer in the slot
			device_context->IASetVertexBuffers(slot, 1, nullptr, nullptr, nullptr);
			return;
		}

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		const Buffer* buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(buffer_handle));
		if (buffer->info.type != BufferType::VERTEX)
		{
			log_error("Buffer system: attempted to set invalid buffer type as vertex buffer!\n");
			return;
		}

		ID3D11Buffer* vertex_buffers[] = { buffer->d3d_buffer.Get() };
		const UINT buffer_stride = (stride < 0) ? buffer->info.element_size : stride;
		const UINT buffer_offset = offset;

		device_context->IASetVertexBuffers(slot, 1, vertex_buffers, &buffer_stride, &buffer_offset);
	}

	void BufferSystem::set_vertex_buffer_slots(const VertexBufferSpan& vertex_buffers)
	{
		temp_buffer_vector.clear();
		temp_buffer_vector.reserve(vertex_buffers.buffers.size());

		static std::vector<UINT> strides;
		strides.clear();
		strides.reserve(vertex_buffers.buffers.size());

		static std::vector<UINT> offsets;
		offsets.clear();
		offsets.reserve(vertex_buffers.buffers.size());

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();

		for (const VertexBufferInfo& current_buffer_info : vertex_buffers.buffers)
		{
			ID3D11Buffer* current_buffer_pointer = nullptr;
			UINT current_stride = 0;
			UINT current_offset = 0;
			if (current_buffer_info.buffer.is_valid() == true)
			{
				const Buffer* current_buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(current_buffer_info.buffer));
				if (current_buffer->info.type != BufferType::VERTEX)
				{
					log_error("Buffer system: attempted to set invalid buffer type as vertex buffer!\n");
					return;
				}

				current_buffer_pointer = current_buffer->d3d_buffer.Get();
				current_stride = (current_buffer_info.stride < 0) ? current_buffer->info.element_size : current_buffer_info.stride;
				current_offset = current_buffer_info.offset;
			}

			temp_buffer_vector.push_back(current_buffer_pointer);
			strides.push_back(current_stride);
			offsets.push_back(current_offset);
		}

		// TODO: keep track of what buffers are set where!
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->IASetVertexBuffers(static_cast<UINT>(vertex_buffers.start_slot), static_cast<UINT>(vertex_buffers.buffers.size()), temp_buffer_vector.data(), strides.data(), offsets.data());
	}

	void BufferSystem::set_index_buffer(BufferHandle buffer_handle, GraphicsAPIDataFormat format, int32_t offset)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		if (buffer_handle.is_valid() == false)
		{
			// Unset the index buffer
			device_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			return;
		}

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		const Buffer* buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(buffer_handle));
		if (buffer->info.type != BufferType::INDEX)
		{
			log_error("Buffer system: attempted to set invalid buffer type as index buffer!\n");
			return;
		}

		device_context->IASetIndexBuffer(buffer->d3d_buffer.Get(), get_dxgi_format(format), offset);
	}

	void BufferSystem::set_constant_buffer(ShaderType shader, BufferHandle buffer_handle, int32_t slot)
	{
		// Invalid handle means we unset the buffer
		ID3D11Buffer* constant_buffers[] = { nullptr };
		if (buffer_handle.is_valid() == true)
		{
			ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
			const Buffer* buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(buffer_handle));
			if (buffer->info.type != BufferType::CONSTANT)
			{
				log_error("Buffer system: attempted to set invalid buffer type as constant buffer!\n");
				return;
			}

			constant_buffers[0] = buffer->d3d_buffer.Get();
		}

		set_constant_buffers(m_graphics_api.get_device_context(), shader, slot, 1, constant_buffers);
	}

	void BufferSystem::set_constant_buffer_slots(ShaderType shader, const ConstantBufferSpan& constant_buffers)
	{
		temp_buffer_vector.clear();
		temp_buffer_vector.reserve(constant_buffers.buffers.size());

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();

		for (const BufferHandle& current_buffer_handle : constant_buffers.buffers)
		{
			ID3D11Buffer* current_buffer_pointer = nullptr;
			if (current_buffer_handle.is_valid() == true)
			{
				const Buffer* buffer = dx_resource_system.get_resource<Buffer>(D3DBufferHandle::from_resource_handle(current_buffer_handle));
				if (buffer->info.type != BufferType::CONSTANT)
				{
					log_error("Buffer system: attempted to set invalid buffer type as constant buffer!\n");
					return;
				}

				current_buffer_pointer = buffer->d3d_buffer.Get();
			}

			temp_buffer_vector.push_back(current_buffer_pointer);
		}

		set_constant_buffers(m_graphics_api.get_device_context(), shader, constant_buffers.start_slot, static_cast<UINT>(constant_buffers.buffers.size()), temp_buffer_vector.data());
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
		log_message("Shutting down Buffer system (DirectX).\n");

		// Clear buffers
		// TODO: add warning in case of leftover buffers?
		m_buffer_pool.reset();

		log_message("Buffer system (DirectX) shut down successfully.\n");
	}
}