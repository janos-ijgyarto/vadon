#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/TextureSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace
{
	constexpr D3D11_RTV_DIMENSION get_d3d_rtv_dimension(Vadon::Render::RenderTargetViewType rtv_type)
	{
		switch (rtv_type)
		{
		case Vadon::Render::RenderTargetViewType::UNKNOWN:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_UNKNOWN;
		case Vadon::Render::RenderTargetViewType::BUFFER:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_BUFFER;
		case Vadon::Render::RenderTargetViewType::TEXTURE_1D:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE1D;
		case Vadon::Render::RenderTargetViewType::TEXTURE_1D_ARRAY:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		case Vadon::Render::RenderTargetViewType::TEXTURE_2D:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
		case Vadon::Render::RenderTargetViewType::TEXTURE_2D_ARRAY:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		case Vadon::Render::RenderTargetViewType::TEXTURE_2D_MS:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DMS;
		case Vadon::Render::RenderTargetViewType::TEXTURE_2D_MS_ARRAY:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		case Vadon::Render::RenderTargetViewType::TEXTURE_3D:
			return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE3D;
		}
		
		return D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_UNKNOWN;
	}

	constexpr D3D11_DSV_DIMENSION get_d3d_dsv_dimension(Vadon::Render::DepthStencilViewType dsv_type)
	{
		switch (dsv_type)
		{
		case Vadon::Render::DepthStencilViewType::UNKNOWN:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_UNKNOWN;
		case Vadon::Render::DepthStencilViewType::TEXTURE_1D:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE1D;
		case Vadon::Render::DepthStencilViewType::TEXTURE_1D_ARRAY:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		case Vadon::Render::DepthStencilViewType::TEXTURE_2D:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		case Vadon::Render::DepthStencilViewType::TEXTURE_2D_ARRAY:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		case Vadon::Render::DepthStencilViewType::TEXTURE_2D_MS:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DMS;
		case Vadon::Render::DepthStencilViewType::TEXTURE_2D_MS_ARRAY:
			return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		}

		return D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_UNKNOWN;
	}

	constexpr UINT get_d3d_dsv_flags(Vadon::Render::DepthStencilViewFlags flags)
	{
		UINT d3d_flags = 0;
		if (flags == Vadon::Render::DepthStencilViewFlags::NONE)
		{
			return d3d_flags;
		}

		if (Vadon::Utilities::to_bool(flags & Vadon::Render::DepthStencilViewFlags::READ_ONLY_DEPTH))
		{
			d3d_flags |= D3D11_DSV_READ_ONLY_DEPTH;
		}

		if (Vadon::Utilities::to_bool(flags & Vadon::Render::DepthStencilViewFlags::READ_ONLY_STENCIL))
		{
			d3d_flags |= D3D11_DSV_READ_ONLY_STENCIL;
		}

		return d3d_flags;
	}
}

namespace Vadon::Private::Render::DirectX
{
	WindowHandle RenderTargetSystem::create_window(const WindowInfo& window_info)
	{
		HWND platform_handle = static_cast<HWND>(window_info.platform_handle);

		// Check existing windows first to make sure we don't add it twice
		for (const WindowPool::ObjectPair& current_window_pair : m_window_pool)
		{
			const Window* current_window = current_window_pair.second;
			if (current_window->hwnd == platform_handle)
			{
				return current_window_pair.first;
			}
		}

		// Create a new window
		// Start with a swap chain
		// FIXME: for D3D, we need to branch on feature level (11.1 supports newer struct)
		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		ZeroMemory(&swap_chain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));

		// If 0, DXGI will query the required dimensions from the window
		swap_chain_desc.BufferDesc.Width = window_info.size.x;
		swap_chain_desc.BufferDesc.Height = window_info.size.y;

		// FIXME: validate format w.r.t hardware just to be safe?
		swap_chain_desc.BufferDesc.Format = get_dxgi_format(window_info.format);

		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0; // Only becomes relevant in exclusive fullscreen, and we are starting in windowed
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;

		swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // Don't even think about these
		swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swap_chain_desc.SampleDesc.Count = 1; // Relevant for MSAA, but cannot be used with flip mode anyway
		swap_chain_desc.SampleDesc.Quality = 0;

		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Always use this

		swap_chain_desc.BufferCount = window_info.buffer_count; // 2 is enough, 3 might be better (need to combine with SetMaximumFrameLatency)

		HWND window_handle = static_cast<HWND>(window_info.platform_handle);
		swap_chain_desc.OutputWindow = window_handle; // Use the HWND
		swap_chain_desc.Windowed = TRUE; // Always start in windowed, programmatically change to fullscreen afterward if needed (TODO: check the HWND to make sure it's in the correct state?)

		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Always use this, FLIP_SEQUENTIAL could be relevant for certain apps, but for games we want DISCARD

		swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // We want to manage mode switching, and we should enable tearing (only becomes relevant in fullscreen)

		// TODO: might need to pick a different "create" function depending on context
		IDXGIFactory6* dxgi_factory = m_graphics_api.get_dxgi_factory();
		ID3D11Device* d3d_device = m_graphics_api.get_device();

		DXGISwapChain swap_chain;
		HRESULT result = dxgi_factory->CreateSwapChain(d3d_device, &swap_chain_desc, swap_chain.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			log_error("GraphicsAPI: unable to create swap chain!\n");
			return WindowHandle();
		}

		// Prevent automatic handling of Alt+Enter
		result = dxgi_factory->MakeWindowAssociation(window_handle, DXGI_MWA_NO_WINDOW_CHANGES);
		if (FAILED(result))
		{
			log_error("GraphicsAPI: unable to create swap chain!\n");
			return WindowHandle();
		}

		// Create a render target for the back buffer
		D3DRenderTargetView back_buffer_view;
		if (create_back_buffer_view(swap_chain, back_buffer_view) == false)
		{
			log_error("GraphicsAPI: unable to create swap chain!\n");
			return WindowHandle();
		}

		// Everything succeeded, create the window and back buffer RTV in the pools
		const WindowHandle new_window_handle = m_window_pool.add();

		Window& new_window = m_window_pool.get(new_window_handle);

		new_window.info = window_info;
		new_window.hwnd = platform_handle;
		new_window.swap_chain = swap_chain;

		new_window.back_buffer_rtv = m_rtv_pool.add();

		RenderTargetView& back_buffer_rtv = m_rtv_pool.get(new_window.back_buffer_rtv);
		back_buffer_rtv.d3d_rt_view = back_buffer_view;

		return new_window_handle;
	}

	WindowInfo RenderTargetSystem::get_window_info(WindowHandle window_handle) const
	{
		const Window& window = m_window_pool.get(window_handle);
		return window.info;
	}
	
	void RenderTargetSystem::update_window(WindowHandle window_handle)
	{
		const Window& selected_window = m_window_pool.get(window_handle);

		// TODO: present parameters?
		selected_window.swap_chain->Present(0, 0);
	}

	void RenderTargetSystem::remove_window(WindowHandle window_handle)
	{
		Window& window = m_window_pool.get(window_handle);

		// Window was found, remove the target
		remove_render_target_view(window.back_buffer_rtv);

		// Unset fullscreen state before we release, just to be safe
		window.swap_chain->SetFullscreenState(false, NULL);
		window.swap_chain.Reset();

		// Remove the window itself
		m_window_pool.remove(window_handle);
	}

	void RenderTargetSystem::resize_window(WindowHandle window_handle, const Vadon::Utilities::Vector2i& window_size)
	{
		Window& window = m_window_pool.get(window_handle);

		// Get dimensions from SC
		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		HRESULT hr = window.swap_chain->GetDesc(&swap_chain_desc);
		if (FAILED(hr))
		{
			log_error("Render target system: unable to get swap chain description!\n");
			return;
		}

		if ((swap_chain_desc.BufferDesc.Width == static_cast<uint32_t>(window_size.x)) && (swap_chain_desc.BufferDesc.Height == static_cast<uint32_t>(window_size.y)))
		{
			// Window size unchanged
			return;
		}

		// Need to resize the swap chain
		// Release any outstanding references to back buffer
		RenderTargetView& back_buffer_target = m_rtv_pool.get(window.back_buffer_rtv);
		back_buffer_target.d3d_rt_view.Reset();

		// Clear state and flush just to be safe
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearState();
		device_context->Flush();

		// Preserve the existing buffer count and format.
		// If width and height are 0, DXGI will automatically determine size via window handle
		hr = window.swap_chain->ResizeBuffers(0, window_size.x, window_size.y, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		if (FAILED(hr))
		{
			log_error("Render target system: failed to resize swap chain buffers!\n");
			return;
		}

		if (create_back_buffer_view(window.swap_chain, back_buffer_target.d3d_rt_view) == false)
		{
			// TODO: handle error!
		}
	}

	RTVHandle RenderTargetSystem::create_render_target_view(ResourceHandle resource_handle, const RenderTargetViewInfo& rtv_info)
	{
		D3D11_RENDER_TARGET_VIEW_DESC d3d_rtv_desc;
		ZeroMemory(&d3d_rtv_desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

		d3d_rtv_desc.Format = get_dxgi_format(rtv_info.format);
		assert(rtv_info.type == RenderTargetViewType::TEXTURE_2D); // FIXME: support other RT types!
		d3d_rtv_desc.ViewDimension = get_d3d_rtv_dimension(rtv_info.type);

		d3d_rtv_desc.Texture2D.MipSlice = rtv_info.type_info.mip_slice;

		D3DRenderTargetView d3d_rtv;

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		Resource* resource = dx_resource_system.get_resource(D3DResourceHandle::from_resource_handle(resource_handle));

		if (internal_create_rtv(d3d_rtv, resource->d3d_resource.Get(), &d3d_rtv_desc) == false)
		{
			log_error("Render target system: failed to create D3D RTV!\n");
			return RTVHandle();
		}

		// Everything succeeded, add to pool
		RTVHandle new_rtv_handle = m_rtv_pool.add();
		RenderTargetView& new_rtv = m_rtv_pool.get(new_rtv_handle);

		new_rtv.info = rtv_info;
		new_rtv.d3d_rt_view = d3d_rtv;
		new_rtv.resource = resource_handle;

		return new_rtv_handle;
	}

	void RenderTargetSystem::remove_render_target_view(RTVHandle rtv_handle)
	{
		// TODO: make sure we check whether the target was set, remove if so
		// TODO2: make sure we can't remove a window RT directly, only if we remove the window itself?
		RenderTargetView& render_target_view = m_rtv_pool.get(rtv_handle);

		// Release the D3D resource
		render_target_view.d3d_rt_view.Reset();

		m_rtv_pool.remove(rtv_handle);
	}

	DSVHandle RenderTargetSystem::create_depth_stencil_view(ResourceHandle resource_handle, const DepthStencilViewInfo& dsv_info)
	{
		GraphicsAPI::Device* d3d_device = m_graphics_api.get_device();

		D3DDepthStencilView d3d_ds_view;
		D3D11_DEPTH_STENCIL_VIEW_DESC d3d_dsv_desc;
		ZeroMemory(&d3d_dsv_desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

		d3d_dsv_desc.Format = get_dxgi_format(dsv_info.format);
		assert(dsv_info.type == DepthStencilViewType::TEXTURE_2D); // FIXME: support other RT types!
		d3d_dsv_desc.ViewDimension = get_d3d_dsv_dimension(dsv_info.type);
		d3d_dsv_desc.Flags = get_d3d_dsv_flags(dsv_info.flags);

		// FIXME: implement proper branching and only set values as needed!
		d3d_dsv_desc.Texture2D.MipSlice = dsv_info.type_info.mip_slice;

		ResourceSystem& dx_resource_system = m_graphics_api.get_directx_resource_system();
		Resource* resource = dx_resource_system.get_resource(D3DResourceHandle::from_resource_handle(resource_handle));

		HRESULT result = d3d_device->CreateDepthStencilView(resource->d3d_resource.Get(), &d3d_dsv_desc, d3d_ds_view.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error?
			return DSVHandle();
		}

		DSVHandle new_dsv_handle = m_dsv_pool.add();
		DepthStencilView& new_depth_stencil_view = m_dsv_pool.get(new_dsv_handle);

		new_depth_stencil_view.info = dsv_info;
		new_depth_stencil_view.d3d_ds_view = d3d_ds_view;

		return new_dsv_handle;
	}
	
	void RenderTargetSystem::remove_depth_stencil_view(DSVHandle dsv_handle)
	{
		DepthStencilView& depth_stencil_view = m_dsv_pool.get(dsv_handle);
		depth_stencil_view.d3d_ds_view.Reset();

		m_dsv_pool.remove(dsv_handle);
	}

	void RenderTargetSystem::set_target(RTVHandle rtv_handle, DSVHandle dsv_handle)
	{
		// TODO: implement support for multiple RTVs!
		ID3D11RenderTargetView* d3d_render_target_views[] = { nullptr };
		ID3D11DepthStencilView* d3d_depth_stencil_view = nullptr;

		// Invalid handle means we unset it
		if (rtv_handle.is_valid() == true)
		{
			const RenderTargetView& render_target_view = m_rtv_pool.get(rtv_handle);
			d3d_render_target_views[0] = render_target_view.d3d_rt_view.Get();
		}

		if (dsv_handle.is_valid() == true)
		{
			const DepthStencilView& depth_stencil_view = m_dsv_pool.get(dsv_handle);
			d3d_depth_stencil_view = depth_stencil_view.d3d_ds_view.Get();
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->OMSetRenderTargets(1, d3d_render_target_views, d3d_depth_stencil_view);
	}

	void RenderTargetSystem::clear_target(RTVHandle rtv_handle, const Vadon::Render::RGBAColor& clear_color)
	{
		const RenderTargetView& render_target_view = m_rtv_pool.get(rtv_handle);

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearRenderTargetView(render_target_view.d3d_rt_view.Get(), &clear_color.x);
	}

	void RenderTargetSystem::clear_depth_stencil(DSVHandle dsv_handle, const DepthStencilClear& clear)
	{
		const DepthStencilView& depth_stencil_view = m_dsv_pool.get(dsv_handle);

		// FIXME: proper utility function!
		const UINT clear_flags = Vadon::Utilities::to_integral(clear.clear_flags);

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearDepthStencilView(depth_stencil_view.d3d_ds_view.Get(), clear_flags, clear.depth, clear.stencil);
	}

	void RenderTargetSystem::apply_viewport(const Vadon::Render::Viewport& viewport)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();

		D3D11_VIEWPORT d3d_viewport;
		memset(&d3d_viewport, 0, sizeof(D3D11_VIEWPORT));
		d3d_viewport.Width = viewport.dimensions.size.x;
		d3d_viewport.Height = viewport.dimensions.size.y;
		d3d_viewport.MinDepth = viewport.depth_min_max.x;
		d3d_viewport.MaxDepth = viewport.depth_min_max.y;
		d3d_viewport.TopLeftX = viewport.dimensions.position.x;
		d3d_viewport.TopLeftY = viewport.dimensions.position.y;

		device_context->RSSetViewports(1, &d3d_viewport);
	}

	RTVHandle RenderTargetSystem::get_window_target(WindowHandle window_handle) const
	{
		const Window& window = m_window_pool.get(window_handle);
		return window.back_buffer_rtv;
	}

	RenderTargetSystem::RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::RenderTargetSystem(core)
		, m_graphics_api(graphics_api)
	{}

	bool RenderTargetSystem::initialize()
	{
		log_message("Initializing Render Target system (DirectX).\n");

		// TODO!!!

		log_message("Render Target system (DirectX) initialized successfully.\n");
		return true;
	}

	void RenderTargetSystem::update()
	{
		// TODO!!!
	}

	void RenderTargetSystem::shutdown()
	{
		log_message("Shutting down Render Target system (DirectX).\n");

		// Clear resources
		// TODO: add warning in case of leftover resources?
		m_rtv_pool.reset();
		m_dsv_pool.reset();
		m_window_pool.reset();

		log_message("Render Target system (DirectX) shut down successfully.\n");
	}

	bool RenderTargetSystem::create_back_buffer_view(DXGISwapChain& swap_chain, D3DRenderTargetView& back_buffer_view)
	{
		// Create a RT for the swap chain back buffer
		ComPtr<ID3D11Texture2D> back_buffer_texture;
		HRESULT result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)back_buffer_texture.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			log_error("Render target system: failed to retrieve window back buffer!\n");			
			return false;
		}

		return internal_create_rtv(back_buffer_view, back_buffer_texture.Get(), nullptr);
	}

	bool RenderTargetSystem::internal_create_rtv(D3DRenderTargetView& d3d_rtv, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* description)
	{
		// Create the render target view with the back buffer pointer
		ID3D11Device* d3d_device = m_graphics_api.get_device();
		HRESULT hr = d3d_device->CreateRenderTargetView(resource, description, d3d_rtv.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			log_error("Render target system: failed to create render target view!\n");
			return false;
		}

		return true;
	}
}