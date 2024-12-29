#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/TextureSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Private::Render::DirectX
{
	WindowHandle RenderTargetSystem::add_window(const WindowInfo& window_info)
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

		// Everything succeeded, create the window and back buffer RT in the pools
		const WindowHandle new_window_handle = m_window_pool.add();

		Window& new_window = m_window_pool.get(new_window_handle);

		new_window.info = window_info;
		new_window.hwnd = platform_handle;
		new_window.swap_chain = swap_chain;

		new_window.back_buffer_rt = m_rt_pool.add();

		RenderTarget& back_buffer_rt = m_rt_pool.get(new_window.back_buffer_rt);
		back_buffer_rt.d3d_rt_view = back_buffer_view;

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
		remove_target(window.back_buffer_rt);

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
		RenderTarget& back_buffer_target = m_rt_pool.get(window.back_buffer_rt);
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

	void RenderTargetSystem::set_window_mode(WindowHandle /*window_handle*/, Vadon::Render::WindowMode /*mode*/)
	{
		// TODO!!!
	}

	RenderTargetHandle RenderTargetSystem::add_target(const Vadon::Render::RenderTargetInfo& /*target_info*/, TextureHandle /*texture_handle*/)
	{
		// TODO: API for creating RT from texture!
		return RenderTargetHandle();
	}

	void RenderTargetSystem::copy_target(RenderTargetHandle /*source_handle*/, RenderTargetHandle /*destination_handle*/)
	{
		// TODO!!!
	}

	void RenderTargetSystem::remove_target(RenderTargetHandle rt_handle)
	{
		// TODO: make sure we check whether the target was set, remove if so
		// TODO2: make sure we can't remove a window RT directly, only if we remove the window itself?
		RenderTarget& render_target = m_rt_pool.get(rt_handle);

		// Release the D3D resource
		render_target.d3d_rt_view.Reset();

		m_rt_pool.remove(rt_handle);
	}

	void RenderTargetSystem::set_target(RenderTargetHandle rt_handle, DepthStencilHandle ds_handle)
	{
		// TODO: allow for multiple RTs!
		ID3D11RenderTargetView* render_target_views[] = { nullptr };
		ID3D11DepthStencilView* depth_stencil_view = nullptr;

		// Invalid handle means we unset it
		if (rt_handle.is_valid() == true)
		{
			const RenderTarget& render_target = m_rt_pool.get(rt_handle);
			render_target_views[0] = render_target.d3d_rt_view.Get();
		}

		if (ds_handle.is_valid() == true)
		{
			const DepthStencil& depth_stencil = m_ds_pool.get(ds_handle);
			depth_stencil_view = depth_stencil.d3d_ds_view.Get();
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->OMSetRenderTargets(1, render_target_views, depth_stencil_view);
	}

	void RenderTargetSystem::clear_target(RenderTargetHandle rt_handle, const Vadon::Render::RGBAColor& clear_color)
	{
		const RenderTarget& render_target = m_rt_pool.get(rt_handle);

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearRenderTargetView(render_target.d3d_rt_view.Get(), &clear_color.x);
	}

	void RenderTargetSystem::clear_depth_stencil(DepthStencilHandle ds_handle, const DepthStencilClear& clear)
	{
		const DepthStencil& depth_stencil = m_ds_pool.get(ds_handle);

		// FIXME: proper utility function!
		const UINT clear_flags = Vadon::Utilities::to_integral(clear.clear_flags);

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearDepthStencilView(depth_stencil.d3d_ds_view.Get(), clear_flags, clear.depth, clear.stencil);
	}

	void RenderTargetSystem::remove_depth_stencil(DepthStencilHandle ds_handle)
	{
		DepthStencil& depth_stencil = m_ds_pool.get(ds_handle);

		// Release the D3D resource
		depth_stencil.d3d_ds_view.Reset();

		m_ds_pool.remove(ds_handle);
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

	RenderTargetHandle RenderTargetSystem::get_window_target(WindowHandle window_handle) const
	{
		const Window& window = m_window_pool.get(window_handle);
		return window.back_buffer_rt;
	}

	DepthStencilHandle RenderTargetSystem::create_depth_stencil_view(const D3DResource& resource, const DepthStencilViewInfo& ds_view_info)
	{
		GraphicsAPI::Device* d3d_device = m_graphics_api.get_device();

		D3DDepthStencilView d3d_ds_view;

		// FIXME: use DS info!
		HRESULT result = d3d_device->CreateDepthStencilView(resource.Get(), NULL, d3d_ds_view.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error?
			return DepthStencilHandle();
		}

		DepthStencilHandle new_ds_handle = m_ds_pool.add();
		DepthStencil& new_depth_stencil = m_ds_pool.get(new_ds_handle);

		new_depth_stencil.info = ds_view_info;
		new_depth_stencil.d3d_ds_view = d3d_ds_view;

		return new_ds_handle;
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
		m_rt_pool.reset();
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

		return internal_create_rt_view(back_buffer_view, back_buffer_texture.Get(), nullptr);
	}

	bool RenderTargetSystem::internal_create_rt_view(D3DRenderTargetView& rt_view, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* description)
	{
		// Create the render target view with the back buffer pointer
		ID3D11Device* d3d_device = m_graphics_api.get_device();
		HRESULT hr = d3d_device->CreateRenderTargetView(resource, description, rt_view.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			log_error("Render target system: failed to create render target view!\n");
			return false;
		}

		return true;
	}
}