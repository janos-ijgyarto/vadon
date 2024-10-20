#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonDemo/Core/GameCore.hpp>

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
			// Add event handler for window move & resize (affects rendering so it has to happen at the appropriate time)
			VadonApp::Platform::PlatformInterface& platform_interface = m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
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
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();

			const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();
			rt_system.update_window(main_window_info.render_handle);
		}

		void process_events()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

			VadonApp::Platform::WindowEvent window_event;
			while (m_window_event_queue.try_dequeue(window_event))
			{
				switch (window_event.type)
				{
				case VadonApp::Platform::WindowEventType::RESIZED:
				{
					// Resize the window render target
					const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

					Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
					rt_system.resize_window(main_window_info.render_handle, main_window_info.window.size);
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