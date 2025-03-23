#include <VadonEditor/Render/RenderSystem.hpp>

#include <VadonEditor/Platform/PlatformInterface.hpp>
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
		Vadon::Render::WindowHandle m_render_window;

		std::vector<Vadon::Render::Canvas::RenderContext> m_canvas_contexts;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{

		}

		bool initialize()
		{
			VadonApp::Platform::WindowHandle main_window = m_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();

			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
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
			// FIXME: draw to separate RT and copy to back buffer at the end!		
			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			const Vadon::Render::RTVHandle main_window_target = rt_system.get_window_back_buffer_view(m_render_window);

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
						rt_system.set_target(main_window_target, Vadon::Render::DSVHandle());
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
								// Get drawable size															
								VadonApp::Platform::WindowHandle main_window = m_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();
								const Vadon::Utilities::Vector2i drawable_size = platform_interface.get_window_drawable_size(main_window);

								// Resize the render window
								Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
								rt_system.resize_window(m_render_window, drawable_size);
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
			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			rt_system.update_window(Vadon::Render::WindowUpdateInfo{ .window = m_render_window });
		}

		void shutdown()
		{
			// TODO!!!
		}
	};

	RenderSystem::~RenderSystem() = default;

	Vadon::Render::WindowHandle RenderSystem::get_render_window() const
	{
		return m_internal->m_render_window;
	}

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