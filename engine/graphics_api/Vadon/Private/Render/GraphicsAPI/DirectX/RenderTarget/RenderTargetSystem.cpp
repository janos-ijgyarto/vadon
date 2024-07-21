#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>

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
		// FIXME: make this configurable?
		// FIXME2: for D3D, we need to branch on feature level (11.1 supports newer struct)
		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		ZeroMemory(&swap_chain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swap_chain_desc.BufferDesc.Width = window_info.dimensions.x;
		swap_chain_desc.BufferDesc.Height = window_info.dimensions.y;

		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

		swap_chain_desc.OutputWindow = platform_handle;

		// TODO:
#if 0
		if (m_vsync_enabled)
		{
			swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
			swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else
		{
			swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
			swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
		}
#else
		// No vsync
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
#endif

		// TODO: configure MSAA
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;

		// TODO: windowed/fullscreen
#if 0
		if (fullscreen)
		{
			swap_chain_desc.Windowed = false;
		}
		else
		{
			swap_chain_desc.Windowed = true;
		}
#endif
		// TODO
		swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// TODO
		swap_chain_desc.BufferCount = 2;
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		// TODO!!!
		swap_chain_desc.Windowed = true;
		//swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow alt-enter switching
		swap_chain_desc.Flags = 0;

		// TODO: might need to pick a different "create" function depending on context
		IDXGIFactory6* dxgi_factory = m_graphics_api.get_dxgi_factory();
		ID3D11Device* d3d_device = m_graphics_api.get_device();

		DXGISwapChain swap_chain;
		HRESULT result = dxgi_factory->CreateSwapChain(d3d_device, &swap_chain_desc, swap_chain.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			log_error("Unable to create window!");
			return WindowHandle();
		}

		// Create a render target for the back buffer
		// FIXME: adjust so it can use the same API as other render targets!!!
		D3DRenderTargetView back_buffer_view;
		if (!update_back_buffer_view(swap_chain, back_buffer_view))
		{
			return WindowHandle();
		}

		// Everything succeeded, create the window and back buffer RT in the pools
		const WindowHandle new_window_handle = m_window_pool.add();

		Window& new_window = m_window_pool.get(new_window_handle);

		new_window.info = window_info;
		new_window.hwnd = platform_handle;
		new_window.swap_chain = swap_chain;

		new_window.back_buffer = m_rt_pool.add();

		RenderTarget& back_buffer_rt = m_rt_pool.get(new_window.back_buffer);
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
		remove_target(window.back_buffer);

		// Unset fullscreen state before we release, just to be safe
		window.swap_chain->SetFullscreenState(false, NULL);
		window.swap_chain.Reset();

		// Remove the window itself
		m_window_pool.remove(window_handle);
	}

	void RenderTargetSystem::resize_window(WindowHandle window_handle, const Vadon::Utilities::Vector2i& window_size)
	{
		Window& window = m_window_pool.get(window_handle);

		// Update window dimensions
		window.info.dimensions = window_size;

		// TODO: unset RT if it happens to be set!
		//ID3D11DeviceContext* device_context = m_graphics_api.get_device_context();
		//device_context->OMSetRenderTargets(0, 0, 0);

		// Release the swap chain render target
		RenderTarget& back_buffer_target = m_rt_pool.get(window.back_buffer);
		back_buffer_target.d3d_rt_view.Reset();

		// Preserve the existing buffer count and format.
		// Automatically choose the width and height to match the client rect for HWNDs.
		const HRESULT result = window.swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(result))
		{
			// TODO: warning/error?
			return;
		}

		if (update_back_buffer_view(window.swap_chain, back_buffer_target.d3d_rt_view) == false)
		{
			// TODO: handle error!
		}
	}

	void RenderTargetSystem::set_window_mode(WindowHandle /*window_handle*/, Vadon::Render::WindowMode /*mode*/)
	{
		// TODO!!!
	}

	RenderTargetHandle RenderTargetSystem::add_target(const Vadon::Render::RenderTargetInfo& /*render_target*/)
	{
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
		return window.back_buffer;
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

	bool RenderTargetSystem::update_back_buffer_view(DXGISwapChain& swap_chain, D3DRenderTargetView& back_buffer_view)
	{
		// FIXME: deduplicate RT creation for RTs unrelated to swap chains
		// Create a RT for the swap chain back buffer
		ComPtr<ID3D11Texture2D> back_buffer_texture;
		HRESULT result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)back_buffer_texture.ReleaseAndGetAddressOf());
		if (SUCCEEDED(result))
		{
			// Create the render target view with the back buffer pointer
			ID3D11Device* d3d_device = m_graphics_api.get_device();
			result = d3d_device->CreateRenderTargetView(back_buffer_texture.Get(), NULL, back_buffer_view.ReleaseAndGetAddressOf());
			if (SUCCEEDED(result))
			{
				return true;
			}
			else
			{
				log_error("Unable to create window render target view!");
			}
		}
		else
		{
			log_error("Unable to get window back buffer!");
		}
		return false;
	}
}