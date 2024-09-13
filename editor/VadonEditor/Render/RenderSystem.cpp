#include <VadonEditor/Render/RenderSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

namespace VadonEditor::Render
{
	struct RenderSystem::Internal
	{
		Core::Editor& m_editor;

		Vadon::Render::FrameGraphHandle m_frame_graph;

		std::vector<Vadon::Render::Canvas::RenderContext> m_canvas_contexts;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{

		}

		bool initialize()
		{
			init_frame_graph();
			register_events();
			return true;
		}

		bool init_frame_graph()
		{
			// FIXME: should not have a fixed frame graph, instead process active viewports and render tasks
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			// Draw to the main window
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			const Vadon::Render::RenderTargetHandle main_window_target = rt_system.get_window_target(main_window_info.render_handle);

			// Create frame graph
			// FIXME: make this even more flexible, maybe even possible to set purely from data (so model doesn't even reference systems explicitly)
			Vadon::Render::FrameGraphInfo frame_graph_info;
			frame_graph_info.targets.emplace_back("main_window");

			// Start with a pass that clears everything and sets shared state
			constexpr const char* clear_pass_target = "main_window_cleared";
			{
				Vadon::Render::RenderPass& clear_pass = frame_graph_info.passes.emplace_back();
				clear_pass.name = "Clear";

				clear_pass.targets.emplace_back("main_window", clear_pass_target);
				clear_pass.execution = [main_window_target, &rt_system]()
					{
						rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.0f, 1.0f));
						rt_system.set_target(main_window_target, Vadon::Render::DepthStencilHandle());
					};
			}

			// Canvas
			constexpr const char* canvas_pass_target = "main_window_canvas";
			{
				Vadon::Render::RenderPass& canvas_pass = frame_graph_info.passes.emplace_back();
				canvas_pass.name = "Canvas";

				canvas_pass.targets.emplace_back(clear_pass_target, canvas_pass_target);

				canvas_pass.execution = [this]()
					{
						Vadon::Render::Canvas::CanvasSystem& canvas_system = m_editor.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
						for (const Vadon::Render::Canvas::RenderContext& current_context : m_canvas_contexts)
						{
							canvas_system.render(current_context);
						}
						m_canvas_contexts.clear();
					};
			}

			// Dev GUI
			constexpr const char* dev_gui_target = "dev_gui";
			{
				Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

				dev_gui_pass.name = "DevGUI";
				dev_gui_pass.targets.emplace_back(canvas_pass_target, dev_gui_target); // FIXME: make sure these pass-related names are accessible so we're not implicitly using the same string!

				UI::UISystem& ui_system = m_editor.get_system<UI::UISystem>();
				VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
				dev_gui_pass.execution = [this, &ui_system, &dev_gui_system]()
					{
						dev_gui_system.render();
					};
			}

			Vadon::Render::FrameSystem& frame_system = m_editor.get_engine_core().get_system<Vadon::Render::FrameSystem>();
			m_frame_graph = frame_system.create_graph(frame_graph_info);

			if (m_frame_graph.is_valid() == false)
			{
				return false;
			}

			return true;
		}

		void register_events()
		{
			// Add event handler for window move & resize (affects rendering so it has to happen at the appropriate time)
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

			platform_interface.register_event_callback(
				[this, &engine_app, &platform_interface](const VadonApp::Platform::PlatformEventList& platform_events)
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
							case VadonApp::Platform::WindowEventType::RESIZED:
							{
								// Resize the render window
								const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

								Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
								rt_system.resize_window(main_window_info.render_handle, Vadon::Utilities::Vector2i(window_event.data1, window_event.data2));
							}
							break;
							}
						}
						break;
						}
					}
				}
			);
		}

		void update()
		{
			// Execute the frame graph
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			Vadon::Render::FrameSystem& frame_system = engine_core.get_system<Vadon::Render::FrameSystem>();
			frame_system.execute_graph(m_frame_graph);

			// Present to the main window
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			rt_system.update_window(main_window_info.render_handle);
		}

		void shutdown()
		{
			// TODO!!!
		}
	};

	RenderSystem::~RenderSystem() = default;

	void RenderSystem::enqueue_canvas(const Vadon::Render::Canvas::RenderContext& context)
	{
		m_internal->m_canvas_contexts.push_back(context);
	}

	RenderSystem::RenderSystem(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>(editor))
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

	void RenderSystem::shutdown()
	{
		m_internal->shutdown();
	}
}