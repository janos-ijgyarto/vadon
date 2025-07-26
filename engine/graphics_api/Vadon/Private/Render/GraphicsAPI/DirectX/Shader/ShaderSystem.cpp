#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/ShaderSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

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
	}

	ShaderHandle ShaderSystem::create_shader(const ShaderInfo& shader_info, const void* shader_data, size_t shader_data_size)
	{
		VADON_ASSERT((shader_data != nullptr) && (shader_data_size > 0), "Invalid shader data!");

		switch (shader_info.type)
		{
		case ShaderType::VERTEX:
			return create_vertex_shader(shader_info, shader_data, shader_data_size);
		case ShaderType::PIXEL:
			return create_pixel_shader(shader_info, shader_data, shader_data_size);
		}

		return ShaderHandle();
	}

	void ShaderSystem::apply_shader(ShaderHandle shader_handle)
	{
		// TODO: be able to unset shaders?
		const Shader& shader = m_shader_pool.get(shader_handle);

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		switch (shader.info.type)
		{
		case ShaderType::VERTEX:
			device_context->VSSetShader(shader.d3d_vertex_shader.Get(), nullptr, 0);
			break;
		case ShaderType::PIXEL:
			device_context->PSSetShader(shader.d3d_pixel_shader.Get(), nullptr, 0);
			break;
			// TODO: other shader types?
		}
	}

	void ShaderSystem::remove_shader(ShaderHandle /*shader_handle*/)
	{
		// TODO!!!
	}

	VertexLayoutHandle ShaderSystem::create_vertex_layout(const VertexLayoutInfo& layout_info, const void* shader_data, size_t shader_data_size)
	{
		VADON_ASSERT((shader_data != nullptr) && (shader_data_size > 0), "Invalid shader data!");

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
			shader_data, shader_data_size, d3d_input_layout.ReleaseAndGetAddressOf()) != S_OK)
		{
			return VertexLayoutHandle();
		}

		// Everything succeeded, add layout to the cache
		const VertexLayoutHandle new_layout_handle = m_layout_pool.add();

		VertexLayout& new_layout = m_layout_pool.get(new_layout_handle);
		new_layout.d3d_input_layout = d3d_input_layout;

		return new_layout_handle;
	}

	void ShaderSystem::set_vertex_layout(VertexLayoutHandle layout_handle)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		// Invalid handle means we unset the layout
		if (layout_handle.is_valid() == false)
		{
			device_context->IASetInputLayout(nullptr);
			return;
		}

		VertexLayout& vertex_layout = m_layout_pool.get(layout_handle);
		device_context->IASetInputLayout(vertex_layout.d3d_input_layout.Get());
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
		log_message("Shutting down Shader system (DirectX).\n");

		// Clear resources
		// TODO: add warning in case of leftover resources?
		m_shader_pool.reset();
		m_layout_pool.reset();

		log_message("Shader system (DirectX) shut down successfully.\n");
	}


	ShaderHandle ShaderSystem::create_vertex_shader(const ShaderInfo& shader_info, const void* shader_data, size_t shader_data_size)
	{
		D3DVertexShader d3d_vertex_shader;
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		HRESULT hr = device->CreateVertexShader(shader_data, shader_data_size, nullptr, d3d_vertex_shader.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			// TODO: error
			return ShaderHandle();
		}

		// Everything succeeded, add the shader to our cache
		const ShaderHandle new_shader_handle = m_shader_pool.add();
	
		Shader& new_shader = m_shader_pool.get(new_shader_handle);
		new_shader.info = shader_info;
		new_shader.d3d_vertex_shader = d3d_vertex_shader;

		return new_shader_handle;
	}

	ShaderHandle ShaderSystem::create_pixel_shader(const ShaderInfo& shader_info, const void* shader_data, size_t shader_data_size)
	{
		D3DPixelShader d3d_pixel_shader;
		GraphicsAPI::Device* device = m_graphics_api.get_device();
		HRESULT hr = device->CreatePixelShader(shader_data, shader_data_size, nullptr, d3d_pixel_shader.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			// TODO: error
			return ShaderHandle();
		}

		// Everything succeeded, add the shader to our cache
		const ShaderHandle new_shader_handle = m_shader_pool.add();

		Shader& new_shader = m_shader_pool.get(new_shader_handle);
		new_shader.info = shader_info;
		new_shader.d3d_pixel_shader = d3d_pixel_shader;

		return new_shader_handle;
	}
}