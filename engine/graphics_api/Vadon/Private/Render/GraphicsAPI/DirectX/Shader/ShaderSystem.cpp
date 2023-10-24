#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

#include <d3dcompiler.h>

namespace Vadon::Private::Render::DirectX
{
	namespace
	{
		constexpr D3D11_INPUT_CLASSIFICATION get_input_classification(VertexElementType type)
		{
			switch (type)
			{
			case VertexElementType::PER_VERTEX:
				return D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			}

			return D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		}

		std::string get_d3d_error_blob_message(const D3DBlob& error_blob)
		{
			const char* error_message_ptr = static_cast<const char*>(error_blob->GetBufferPointer());
			return std::string(error_message_ptr, error_message_ptr + error_blob->GetBufferSize());
		}

		const char* get_shader_target(const ShaderInfo& shader_info)
		{
			// FIXME: take into account the supported feature level, optionally allow client code to select target
			switch (shader_info.type)
			{
			case ShaderType::VERTEX:
				return "vs_4_0";
			case ShaderType::PIXEL:
				return "ps_4_0";
			}

			return nullptr;
		}

		using D3DShaderMacroList = std::vector<D3D_SHADER_MACRO>;
		D3DShaderMacroList get_shader_macros(const ShaderInfo& shader_info)
		{
			D3DShaderMacroList shader_macros;
			shader_macros.reserve(shader_info.defines.size() + 1);

			for (const ShaderDefine& current_define : shader_info.defines)
			{
				D3D_SHADER_MACRO& current_macro = shader_macros.emplace_back();
				ZeroMemory(&current_macro, sizeof(D3D_SHADER_MACRO));

				current_macro.Name = current_define.name.c_str();
				current_macro.Definition = current_define.value.c_str();
			}

			D3D_SHADER_MACRO& end_macro = shader_macros.emplace_back();
			ZeroMemory(&end_macro, sizeof(D3D_SHADER_MACRO));

			return shader_macros;
		}
	}

	ShaderHandle ShaderSystem::create_shader(const ShaderInfo& shader_info)
	{
		switch (shader_info.type)
		{
		case ShaderType::VERTEX:
			return create_vertex_shader(shader_info);
		case ShaderType::PIXEL:
			return create_pixel_shader(shader_info);
		}

		return ShaderHandle();
	}

	void ShaderSystem::apply_shader(ShaderHandle shader_handle)
	{
		// TODO: be able to unset shaders?
		Shader* shader = m_shader_pool.get(shader_handle);
		if (!shader)
		{
			// TODO: warning?
			return;
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		switch (shader->info.type)
		{
		case ShaderType::VERTEX:
			device_context->VSSetShader(shader->d3d_vertex_shader.Get(), nullptr, 0);
			break;
		case ShaderType::PIXEL:
			device_context->PSSetShader(shader->d3d_pixel_shader.Get(), nullptr, 0);
			break;
			// TODO: other shader types?
		}

		m_current_shaders[Vadon::Utilities::to_integral(shader->info.type)] = shader_handle;
	}

	void ShaderSystem::remove_shader(ShaderHandle /*shader_handle*/)
	{
		// TODO!!!
	}

	VertexLayoutHandle ShaderSystem::create_vertex_layout(ShaderHandle shader_handle, const VertexLayoutInfo& layout_info)
	{
		Shader* shader = m_shader_pool.get(shader_handle);
		if (!shader)
		{
			// TODO: warning/error?
			return VertexLayoutHandle();
		}

		// FIXME: can ILs be used on anything other than a vertex shader?
		if (shader->info.type != ShaderType::VERTEX)
		{
			// TODO: warning/error?
			return VertexLayoutHandle();
		}

		// Process the element descriptions
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_descs(layout_info.size());

		auto input_element_it = input_element_descs.begin();
		for (const VertexLayoutElement& current_element : layout_info)
		{
			D3D11_INPUT_ELEMENT_DESC& current_element_desc = *input_element_it;
			ZeroMemory(&current_element_desc, sizeof(D3D11_INPUT_ELEMENT_DESC));

			current_element_desc.SemanticName = current_element.name.c_str();
			current_element_desc.SemanticIndex = 0;
			current_element_desc.Format = get_dxgi_format(current_element.format);
			current_element_desc.InputSlot = 0;
			current_element_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			current_element_desc.InputSlotClass = get_input_classification(current_element.type);

			++input_element_it;
		}

		GraphicsAPI::Device* device = m_graphics_api.get_device();
		D3DInputLayout d3d_input_layout;

		if (device->CreateInputLayout(input_element_descs.data(), static_cast<UINT>(input_element_descs.size()),
			shader->d3d_shader_blob->GetBufferPointer(), shader->d3d_shader_blob->GetBufferSize(), d3d_input_layout.ReleaseAndGetAddressOf()) != S_OK)
		{
			return VertexLayoutHandle();
		}

		// Everything succeeded, add layout to the cache
		const VertexLayoutHandle new_layout_handle = m_layout_pool.add();

		VertexLayout* new_layout = m_layout_pool.get(new_layout_handle);
		new_layout->shader = shader_handle;
		new_layout->d3d_input_layout = d3d_input_layout;

		return new_layout_handle;
	}

	void ShaderSystem::set_vertex_layout(VertexLayoutHandle layout_handle)
	{
		ShaderHandle current_vshader_handle = m_current_shaders[Vadon::Utilities::to_integral(ShaderType::VERTEX)];
		if (current_vshader_handle.is_valid() == false)
		{
			// No shader set!
			// FIXME: error message?
			return;
		}

		if (layout_handle.is_valid() == false)
		{
			// Assume we want to unset the input layout
			GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
			device_context->IASetInputLayout(nullptr);
			return;
		}

		VertexLayout* vertex_layout = m_layout_pool.get(layout_handle);
		if (vertex_layout == nullptr)
		{
			// TODO: warning/error?
			return;
		}

		if (vertex_layout->shader != current_vshader_handle)
		{
			// Cannot set layout to a different shader!
			// FIXME: error message?
			return;
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->IASetInputLayout(vertex_layout->d3d_input_layout.Get());
	}

	ResourceViewInfo ShaderSystem::get_resource_view_info(ResourceViewHandle resource_view_handle) const
	{
		const ShaderResourceView* resource = m_resource_pool.get(resource_view_handle);
		if (resource == nullptr)
		{
			return ResourceViewInfo();
		}

		return resource->info;
	}

	void ShaderSystem::apply_resource(ShaderType shader_type, ResourceViewHandle resource_handle, int32_t slot)
	{
		ShaderResourceView* resource = m_resource_pool.get(resource_handle);
		ID3D11ShaderResourceView* resource_array[] = { resource ? resource->d3d_srv.Get() : nullptr };

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		switch (shader_type)
		{
		case ShaderType::VERTEX:
			device_context->VSSetShaderResources(slot, 1, resource_array);
			break;
		case ShaderType::PIXEL:
			device_context->PSSetShaderResources(slot, 1, resource_array);
			break;
		}
	}

	void ShaderSystem::remove_resource(ResourceViewHandle resource_view_handle)
	{
		ShaderResourceView* resource = m_resource_pool.get(resource_view_handle);
		if (!resource)
		{
			return;
		}

		resource->d3d_srv.Reset();
		m_resource_pool.remove(resource_view_handle);
	}

	ResourceViewHandle ShaderSystem::add_resource_view(D3DShaderResourceView d3d_srv)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_description;
		ZeroMemory(&srv_description, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		d3d_srv->GetDesc(&srv_description);

		ResourceViewInfo srv_info = get_srv_info(srv_description);

		return add_resource_view_internal(d3d_srv, srv_info);
	}

	ResourceViewHandle ShaderSystem::create_resource_view(ID3D11Resource* resource, const ResourceViewInfo& resource_view_info)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_description;
		ZeroMemory(&srv_description, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		srv_description.Format = get_dxgi_format(resource_view_info.format);
		srv_description.ViewDimension = get_d3d_srv_dimension(resource_view_info.type);

		switch (resource_view_info.type)
		{
		case ResourceType::BUFFER:
		{
			srv_description.Buffer.FirstElement = resource_view_info.type_info.first_array_slice;
			srv_description.Buffer.NumElements = resource_view_info.type_info.array_size;
		}
		break;
		case ResourceType::TEXTURE_2D:
		{
			srv_description.Texture2D.MipLevels = resource_view_info.type_info.mip_levels;
			srv_description.Texture2D.MostDetailedMip = resource_view_info.type_info.most_detailed_mip;
		}
		break;
		}

		D3DShaderResourceView d3d_srv;

		GraphicsAPI::Device* device = m_graphics_api.get_device();
		const HRESULT result = device->CreateShaderResourceView(resource, &srv_description, d3d_srv.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error message
			return ResourceViewHandle();
		}

		return add_resource_view_internal(d3d_srv, resource_view_info);
	}

	ResourceViewInfo ShaderSystem::get_srv_info(D3D11_SHADER_RESOURCE_VIEW_DESC& srv_desc)
	{
		ResourceViewInfo srv_info;
		srv_info.format = get_graphics_api_data_format(srv_desc.Format);
		srv_info.type = get_srv_type(srv_desc.ViewDimension);

		switch (srv_info.type)
		{
		case ResourceType::TEXTURE_2D:
		{
			srv_info.type_info.mip_levels = srv_desc.Texture2D.MipLevels;
			srv_info.type_info.most_detailed_mip = srv_desc.Texture2D.MostDetailedMip;
		}
		break;
		}

		return srv_info;
	}

	ShaderSystem::ShaderSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::ShaderSystem(core)
		, m_graphics_api(graphics_api)
	{
	}

	bool ShaderSystem::initialize()
	{
		// TODO: anything?
		return true;
	}

	void ShaderSystem::shutdown()
	{
		log("Shutting down Shader system (DirectX).\n");

		// Clear resources
		// TODO: add warning in case of leftover resources?
		m_shader_pool.reset();
		m_layout_pool.reset();
		m_resource_pool.reset();

		log("Shader system (DirectX) shut down successfully.\n");
	}


	ShaderHandle ShaderSystem::create_vertex_shader(const ShaderInfo& shader_info)
	{
		D3DBlob new_shader_blob = compile_shader(shader_info);
		if (new_shader_blob == nullptr)
		{
			return ShaderHandle();
		}

		D3DVertexShader d3d_vertex_shader;
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		if (device->CreateVertexShader(new_shader_blob->GetBufferPointer(), new_shader_blob->GetBufferSize(), nullptr, d3d_vertex_shader.ReleaseAndGetAddressOf()) != S_OK)
		{
			// TODO: error
			return ShaderHandle();
		}

		// Everything succeeded, add the shader to our cache
		const ShaderHandle new_shader_handle = m_shader_pool.add();
	
		Shader* new_shader = m_shader_pool.get(new_shader_handle);
		new_shader->info = shader_info;
		new_shader->d3d_shader_blob = new_shader_blob;
		new_shader->d3d_vertex_shader = d3d_vertex_shader;

		return new_shader_handle;
	}

	ShaderHandle ShaderSystem::create_pixel_shader(const ShaderInfo& shader_info)
	{
		D3DBlob new_shader_blob = compile_shader(shader_info);
		if (new_shader_blob == nullptr)
		{
			return ShaderHandle();
		}

		D3DPixelShader d3d_pixel_shader;
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		if (device->CreatePixelShader(new_shader_blob->GetBufferPointer(), new_shader_blob->GetBufferSize(), nullptr, d3d_pixel_shader.ReleaseAndGetAddressOf()) != S_OK)
		{
			// TODO: error
			return ShaderHandle();
		}

		// Everything succeeded, add the shader to our cache
		const ShaderHandle new_shader_handle = m_shader_pool.add();

		Shader* new_shader = m_shader_pool.get(new_shader_handle);
		new_shader->info = shader_info;
		new_shader->d3d_shader_blob = new_shader_blob;
		new_shader->d3d_pixel_shader = d3d_pixel_shader;

		return new_shader_handle;
	}

	D3DBlob ShaderSystem::compile_shader(const ShaderInfo& shader_info)
	{
		D3DBlob shader_blob;
		D3DBlob error_blob;

		UINT compile_flags = 0;
#ifndef NDEBUG
		compile_flags |= D3DCOMPILE_DEBUG;
#endif

		const char* source_data = shader_info.source.c_str();
		const char* source_name = (shader_info.name.empty() == false) ? shader_info.name.c_str() : nullptr;
		
		D3DShaderMacroList shader_macros;
		if (shader_info.defines.empty() == false)
		{
			shader_macros = get_shader_macros(shader_info);
		}
		const D3D_SHADER_MACRO* shader_macro_ptr = (shader_macros.empty() == false) ? shader_macros.data() : nullptr;

		ID3DInclude* d3d_include = D3D_COMPILE_STANDARD_FILE_INCLUDE; // FIXME: allow custom includes?

		const char* shader_target = get_shader_target(shader_info);

		HRESULT result = D3DCompile(source_data, strlen(shader_info.source.c_str()), source_name, shader_macro_ptr, d3d_include, shader_info.entrypoint.c_str(), shader_target, compile_flags, 0, shader_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			if (error_blob != nullptr)
			{
				error(get_d3d_error_blob_message(error_blob));
			}
			return nullptr;
		}

		return shader_blob;
	}

	ResourceViewHandle ShaderSystem::add_resource_view_internal(D3DShaderResourceView d3d_srv, const ResourceViewInfo& srv_info)
	{
		ResourceViewHandle new_resource_handle = m_resource_pool.add();

		ShaderResourceView* new_resource = m_resource_pool.get(new_resource_handle);
		new_resource->info = srv_info;
		new_resource->d3d_srv = d3d_srv;

		return new_resource_handle;
	}
}