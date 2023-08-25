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
			error("Unable to create window!");
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

		Window* new_window = m_window_pool.get(new_window_handle);

		new_window->info = window_info;
		new_window->hwnd = platform_handle;
		new_window->swap_chain = swap_chain;

		new_window->back_buffer = m_rt_pool.add();

		RenderTarget* back_buffer_rt = m_rt_pool.get(new_window->back_buffer);
		back_buffer_rt->d3d_rt_view = back_buffer_view;

		return new_window_handle;
	}

	WindowInfo RenderTargetSystem::get_window_info(WindowHandle window_handle)
	{
		if (Window* window = m_window_pool.get(window_handle))
		{
			return window->info;
		}

		return Vadon::Render::WindowInfo();
	}
	
	void RenderTargetSystem::update_window(WindowHandle window_handle)
	{
		const Window* selected_window = m_window_pool.get(window_handle);
		if (!selected_window)
		{
			return;
		}

		// TODO: present parameters?
		selected_window->swap_chain->Present(0, 0);
	}

	void RenderTargetSystem::remove_window(WindowHandle window_handle)
	{
		Window* window = m_window_pool.get(window_handle);
		if (!window)
		{
			// Warn about attempting to remove nonexistent window
			warning(std::format("Render target system attempted to remove window with handle \"{0}\", but was not found!", window_handle.handle.to_uint()));
			return;
		}

		// Window was found, remove the target
		remove_target(window->back_buffer);

		// Unset fullscreen state before we release, just to be safe
		window->swap_chain->SetFullscreenState(false, NULL);
		window->swap_chain.Reset();

		// Remove the window itself
		m_window_pool.remove(window_handle);
	}

	void RenderTargetSystem::resize_window(WindowHandle window_handle, const Vadon::Utilities::Vector2i& window_size)
	{
		Window* window = m_window_pool.get(window_handle);
		if (!window)
		{
			// TODO: warning?
			return;
		}

		// Update window dimensions
		window->info.dimensions = window_size;

		// TODO: unset RT if it happens to be set!
		//ID3D11DeviceContext* device_context = m_graphics_api.get_device_context();
		//device_context->OMSetRenderTargets(0, 0, 0);

		// Release the swap chain render target
		RenderTarget* back_buffer_target = m_rt_pool.get(window->back_buffer);
		back_buffer_target->d3d_rt_view.Reset();

		// Preserve the existing buffer count and format.
		// Automatically choose the width and height to match the client rect for HWNDs.
		HRESULT result = window->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(result))
		{
			// TODO: warning/error?
			return;
		}

		if (!update_back_buffer_view(window->swap_chain, back_buffer_target->d3d_rt_view))
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
		RenderTarget* render_target = m_rt_pool.get(rt_handle);
		if (!render_target)
		{
			// Attempting to remove nonexistent RT
			warning(std::format("Render target system attempted to remove target with ID \"{0}\", but was not found!", rt_handle.handle.to_uint()));
			return;
		}

		// Release the D3D resource
		render_target->d3d_rt_view.Reset();

		m_rt_pool.remove(rt_handle);
	}

	void RenderTargetSystem::set_target(RenderTargetHandle rt_handle)
	{
		// TODO: allow for multiple RTs!
		ID3D11RenderTargetView* render_target_views[] = { nullptr };

		// If we find such a RT, we set it. If not found, we assume user wanted to unset the RT
		RenderTarget* render_target = m_rt_pool.get(rt_handle);
		if (render_target)
		{
			render_target_views[0] = render_target->d3d_rt_view.Get();
		}

		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->OMSetRenderTargets(1, render_target_views, nullptr);
	}

	void RenderTargetSystem::clear_target(RenderTargetHandle rt_handle, const Vadon::Render::RGBAColor& clear_color)
	{
		RenderTarget* render_target = m_rt_pool.get(rt_handle);
		if (!render_target)
		{
			// TODO: warning?
			return;
		}

		ID3D11DeviceContext* device_context = m_graphics_api.get_device_context();
		device_context->ClearRenderTargetView(render_target->d3d_rt_view.Get(), &clear_color.x);
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

	RenderTargetHandle RenderTargetSystem::get_window_target(WindowHandle window_handle)
	{
		Window* window = m_window_pool.get(window_handle);
		if (!window)
		{
			return RenderTargetHandle();
		}

		return window->back_buffer;
	}

	RenderTargetSystem::RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::RenderTargetSystem(core)
		, m_graphics_api(graphics_api)
	{}

	bool RenderTargetSystem::initialize()
	{
		log("Initializing Render Target system (DirectX).\n");

		// TODO!!!

		log("Render Target system (DirectX) initialized successfully.\n");
		return true;
	}

	void RenderTargetSystem::update()
	{
		// TODO!!!
	}

	void RenderTargetSystem::shutdown()
	{
		log("Shutting down Render Target system (DirectX).\n");

		// Clear resources
		// TODO: add warning in case of leftover resources?
		m_rt_pool.reset();
		m_window_pool.reset();

		log("Render Target system (DirectX) shut down successfully.\n");
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
				error("Unable to create window render target view!");
			}
		}
		else
		{
			error("Unable to get window back buffer!");
		}
		return false;
	}
}