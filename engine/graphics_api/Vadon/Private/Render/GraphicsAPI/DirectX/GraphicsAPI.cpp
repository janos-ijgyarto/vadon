#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Buffer/BufferSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Pipeline/PipelineSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/ShaderSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/TextureSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#ifndef NDEBUG
#include <dxgidebug.h>
#endif

namespace Vadon::Private::Render::DirectX
{
	typedef HRESULT(WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);

	struct GraphicsAPI::Internal
	{
		BufferSystem m_buffer_system;
		PipelineSystem m_pipeline_system;
		RenderTargetSystem m_rt_system;
		ResourceSystem m_resource_system;
		ShaderSystem m_shader_system;
		TextureSystem m_texture_system;

#ifndef NDEBUG
		ComPtr<IDXGIInfoQueue> m_dxgi_info_queue;
#endif

		Internal(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
			: m_buffer_system(core, graphics_api)
			, m_pipeline_system(core, graphics_api)
			, m_rt_system(core, graphics_api)
			, m_resource_system(core, graphics_api)
			, m_shader_system(core, graphics_api)
			, m_texture_system(core, graphics_api)
		{

		}

		bool initialize()
		{
			// Initialize subsystems
			if (m_resource_system.initialize() == false)
			{
				return false;
			}

			if (!m_buffer_system.initialize())
			{
				return false;
			}

			if (!m_pipeline_system.initialize())
			{
				return false;
			}

			if (!m_rt_system.initialize())
			{
				return false;
			}

			if (!m_shader_system.initialize())
			{
				return false;
			}

			if (!m_texture_system.initialize())
			{
				return false;
			}

			return true;
		}

		void shutdown()
		{
			m_texture_system.shutdown();
			m_shader_system.shutdown();
			m_rt_system.shutdown();
			m_pipeline_system.shutdown();
			m_buffer_system.shutdown();
			m_resource_system.shutdown();
		}
	};

	GraphicsAPI::GraphicsAPI(Vadon::Core::EngineCoreInterface& core)
		: GraphicsAPIBase(core)
		, m_internal(std::make_unique<Internal>(core, *this))
	{
	}

	GraphicsAPI::~GraphicsAPI() = default;

	void GraphicsAPI::draw(const Vadon::Render::DrawCommand& command)
	{
		switch (command.type)
		{
		case Vadon::Render::DrawCommandType::DEFAULT:
			m_device_context->Draw(command.vertices.count, command.vertices.offset);
			break;
		case Vadon::Render::DrawCommandType::INDEXED:
			m_device_context->DrawIndexed(command.indices.count, command.indices.offset, command.vertices.offset);
			break;
		case Vadon::Render::DrawCommandType::INSTANCED:
			m_device_context->DrawInstanced(command.vertices.count, command.instances.count, command.vertices.offset, command.instances.offset);
			break;
		case Vadon::Render::DrawCommandType::INDEXED_INSTANCED:
			m_device_context->DrawIndexedInstanced(command.indices.count, command.instances.count, command.indices.offset, command.vertices.offset, command.instances.offset);
			break;
		}
	}

	bool GraphicsAPI::initialize()
	{
		// FIXME: should implement System interface, where they have this stuff wrapped as "system_log/error/etc."
		log_message("Initializing DirectX graphics API.\n");

		std::array<D3D_FEATURE_LEVEL, 1> feature_levels = {
			D3D_FEATURE_LEVEL_11_1,
			//D3D_FEATURE_LEVEL_11_0
		};

		constexpr UINT num_feature_levels = static_cast<UINT>(feature_levels.size());

		// Create the DXGI factory
		// TODO: if we want to support ancient OS's and/or versions, have fallbacks that try the older functions
		// TODO2: use DXGI debug interface to get additional debug info ( https://walbourn.github.io/dxgi-debug-device/ )
		{
			UINT dxgi_flags = 0;
			HRESULT result;
#ifndef NDEBUG
			dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG;

			result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_internal->m_dxgi_info_queue.ReleaseAndGetAddressOf()));
			if (SUCCEEDED(result))
			{
				m_internal->m_dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
				m_internal->m_dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			}
#endif
			result = CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(&m_dxgi_factory));
			if (FAILED(result))
			{
				log_error("Failed to create DXGI factory!\n");
				return false;
			}
		}

		// Enumerate adapters to decide which one we want to use
		UINT device_flags = 0u;
#ifndef NDEBUG
		device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		for (UINT current_adapter_index = 0; DXGI_ERROR_NOT_FOUND != m_dxgi_factory->EnumAdapterByGpuPreference(current_adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_dxgi_adapter)); ++current_adapter_index)
		{
			DXGI_ADAPTER_DESC1 dxgi_adapter_desc;
			m_dxgi_adapter->GetDesc1(&dxgi_adapter_desc);

			if (dxgi_adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// TODO: allow software drivers (for whatever reason)?
				continue;
			}

			// FIXME: add utility functions for Windows to convert from wide strings
			std::array<char, 260> adapter_desc;
			char default_char = ' ';
			WideCharToMultiByte(CP_ACP, 0, dxgi_adapter_desc.Description, -1, adapter_desc.data(), static_cast<int>(adapter_desc.size()), &default_char, NULL);

			ComPtr<ID3D11Device> device;
			ComPtr<ID3D11DeviceContext>  device_context;

			HRESULT result = D3D11CreateDevice(m_dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, device_flags, feature_levels.data(), num_feature_levels, D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), &m_d3d_info.m_feature_level, device_context.ReleaseAndGetAddressOf());
			if (FAILED(result))
			{
				log_error(std::format("Failed to create D3D11 device: {0}!\n", adapter_desc.data()));
			}
			else
			{
				log_message(std::format("Successfully created D3D11 device: {0}!\n", adapter_desc.data()));

				if (FAILED(device.As(&m_device)))
				{
					log_error("Failed to cast D3D11 device to feature level!\n");
					return false;
				}

				if (FAILED(device_context.As(&m_device_context)))
				{
					log_error("Failed to cast D3D11 device context to feature level!\n");
					return false;
				}

				break;
			}
		}

		if (!m_device)
		{
			log_error("Unable to create D3D11 device!\n");
			return false;
		}

#ifndef NDEBUG
		if (SUCCEEDED(m_device.As(&m_d3d_debug)))
		{
			if (SUCCEEDED(m_device.As(&m_info_queue)))
			{
				// TODO: make these settings adjustable?
				m_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, false);
				m_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				m_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			}
			else
			{
				log_warning("Unable to create D3D info queue!\n");
			}
		}
		else
		{
			log_warning("Unable to create D3D info queue!\n");
		}
#endif
		log_message("DirectX initialized successfully.\n");

		return m_internal->initialize();
	}

	void GraphicsAPI::update()
	{
		// TODO!!!
	}

	void GraphicsAPI::shutdown()
	{
		log_message("Shutting down DirectX graphics API.\n");

		m_internal->shutdown();

		m_device_context->ClearState();
		m_device_context->Flush();

		m_device_context.Reset();
#ifndef NDEBUG
		if (m_info_queue)
		{
			m_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, false);
		}
		if (m_d3d_debug)
		{
			m_d3d_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
			m_d3d_debug.Reset();
		}
		if (m_info_queue)
		{
			D3D11_MESSAGE_CATEGORY hide_categories[] =
			{
				D3D11_MESSAGE_CATEGORY_STATE_CREATION,
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumCategories = _countof(hide_categories);
			filter.DenyList.pCategoryList = hide_categories;

			HRESULT result = m_info_queue->AddStorageFilterEntries(&filter);
			if (FAILED(result))
			{
				log_error("Unable to add D3D info queue storage filter entries!");
			}

			m_info_queue.Reset();
		}
#endif
		m_device.Reset();

		m_dxgi_adapter.Reset();
		m_dxgi_factory.Reset();

		log_message("DirectX successfully shut down.\n");
	}

	ResourceSystem& GraphicsAPI::get_directx_resource_system()
	{
		return m_internal->m_resource_system;
	}
}