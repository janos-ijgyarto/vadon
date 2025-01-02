#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonDemo/Core/GameCore.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Utilities/Container/Concurrent/ConcurrentQueue.hpp>

namespace VadonDemo::Render
{
	struct RenderSystem::Internal
	{
		Core::GameCore& m_game_core;
		Vadon::Render::FrameGraphHandle m_frame_graph;

		Vadon::Render::WindowHandle m_render_window;

		Vadon::Utilities::ConcurrentQueue<VadonApp::Platform::WindowEvent> m_window_event_queue;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{

		}

		void set_frame_graph(const Vadon::Render::FrameGraphInfo& graph_info)
		{
			Vadon::Render::FrameSystem& frame_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::FrameSystem>();
			if (m_frame_graph.is_valid())
			{
				frame_system.remove_graph(m_frame_graph);
			}

			m_frame_graph = frame_system.create_graph(graph_info);
		}

		bool initialize()
		{
			VadonApp::Platform::WindowHandle main_window = m_game_core.get_platform_interface().get_main_window();

			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

			Vadon::Render::WindowInfo render_window_info;
			render_window_info.platform_handle = platform_interface.get_platform_window_handle(main_window);
			render_window_info.format = Vadon::Render::GraphicsAPIDataFormat::B8G8R8A8_UNORM;

			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			m_render_window = rt_system.create_window(render_window_info);

			if (m_render_window.is_valid() == false)
			{
				return false;
			}

			// Add event handler for window move & resize (affects rendering so it has to happen at the appropriate time)
			platform_interface.register_event_callback(
				[this](const VadonApp::Platform::PlatformEventList& platform_events)
				{
					for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
					{
						const VadonApp::Platform::PlatformEventType current_event_type = Vadon::Utilities::to_enum<VadonApp::Platform::PlatformEventType>(static_cast<int32_t>(current_event.index()));
						switch (current_event_type)
						{
						case VadonApp::Platform::PlatformEventType::WINDOW:
						{
							const VadonApp::Platform::WindowEvent& window_event = std::get<VadonApp::Platform::WindowEvent>(current_event);
							switch (window_event.type)
							{
							case VadonApp::Platform::WindowEventType::MOVED:
							case VadonApp::Platform::WindowEventType::RESIZED:
							case VadonApp::Platform::WindowEventType::SIZE_CHANGED:
							{
								// These window events must be handled in the render task context!
								m_window_event_queue.enqueue(window_event);
							}
							break;
							}
						}
						break;
						}
					}
				}
			);

			return true;
		}

		void update()
		{
			if (!m_frame_graph.is_valid())
			{
				// No valid frame graph to draw
				return;
			}

			// Process any events that might have come in
			process_events();

			// FIXME: split this up so it can be done in multiple tasks?
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			// Execute frame graph
			Vadon::Render::FrameSystem& frame_system = engine_core.get_system<Vadon::Render::FrameSystem>();
			frame_system.execute_graph(m_frame_graph);

			// Present to the main window
			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			rt_system.update_window(m_render_window);
		}

		void process_events()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();

			VadonApp::Platform::WindowEvent window_event;
			while (m_window_event_queue.try_dequeue(window_event))
			{
				switch (window_event.type)
				{
				case VadonApp::Platform::WindowEventType::RESIZED:
				case VadonApp::Platform::WindowEventType::SIZE_CHANGED:
				{
					// Get drawable size															
					VadonApp::Platform::WindowHandle main_window = m_game_core.get_platform_interface().get_main_window();
					const Vadon::Utilities::Vector2i drawable_size = engine_app.get_system<VadonApp::Platform::PlatformInterface>().get_window_drawable_size(main_window);

					// Resize the window render target
					Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
					rt_system.resize_window(m_render_window, drawable_size);
				}
					break;
				}
			}
		}
	};

	RenderSystem::~RenderSystem() = default;

	void RenderSystem::set_frame_graph(const Vadon::Render::FrameGraphInfo& graph_info)
	{
		m_internal->set_frame_graph(graph_info);
	}

	Vadon::Render::WindowHandle RenderSystem::get_render_window() const
	{
		return m_internal->m_render_window;
	}

	RenderSystem::RenderSystem(Core::GameCore& game_core)
		: m_internal(std::make_unique<Internal>(game_core))
	{

	}

	bool RenderSystem::initialize()
	{
		return m_internal->initialize();
	}

	void RenderSystem::update()
	{
		m_internal->update();
	}
}