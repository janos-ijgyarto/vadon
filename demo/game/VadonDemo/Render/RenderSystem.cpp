#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/GameCore.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/UI/UISystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/File/Path.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/Frame/FrameSystem.hpp>

#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

namespace VadonDemo::Render
{
	RenderSystem::~RenderSystem() = default;

	Vadon::Render::Canvas::RenderContext& RenderSystem::get_canvas_context()
	{
		Render& common_render = m_game_core.get_core().get_render();
		return common_render.get_context(m_canvas_context);
	}

	TextureResource* RenderSystem::get_texture_resource(std::string_view path)
	{
		// FIXME: deduplicate from Editor implementation
		const std::string path_string(path);
		auto texture_it = m_textures.find(path_string);
		if (texture_it == m_textures.end())
		{
			TextureResource new_resource = m_game_core.get_core().get_render().load_texture_resource(Vadon::Core::FileSystemPath{ .root_directory =	m_game_core.get_project_root_dir(), .path = std::string(path)});

			if (new_resource.texture.is_valid() == false)
			{
				return nullptr;
			}

			// Add to lookup
			texture_it = m_textures.insert(std::make_pair(path_string, new_resource)).first;
		}

		return &texture_it->second;
	}

	RenderSystem::RenderSystem(Core::GameCore& game_core)
		: m_game_core(game_core)
	{

	}

	bool RenderSystem::initialize()
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
							// Get drawable size							
							VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
							VadonApp::Platform::WindowHandle main_window = m_game_core.get_platform_interface().get_main_window();
							const Vadon::Utilities::Vector2i drawable_size = engine_app.get_system<VadonApp::Platform::PlatformInterface>().get_window_drawable_size(main_window);

							// Resize the window render target
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

		if (init_frame_graph() == false)
		{
			return false;
		}

		if (init_canvas_context() == false)
		{
			return false;
		}

		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		ecs_world.get_component_manager().register_event_callback<VadonDemo::Render::CanvasComponent>(
			[this](const Vadon::ECS::ComponentEvent& component_event)
			{
				switch (component_event.event_type)
				{
				case Vadon::ECS::ComponentEventType::ADDED:
					m_deferred_init_queue.push_back(component_event.owner);
					break;
				case Vadon::ECS::ComponentEventType::REMOVED:
					remove_entity(component_event.owner);
					break;
				}
			}
		);

		return true;
	}

	bool RenderSystem::init_frame_graph()
	{
		// TODO: revise this!
		// Allow other systems to register "passes" or "renderers" (TODO: terminology)
		// These are identified by name, can be referenced from the frame graph
		// This system can be data-driven later, for now we use a hardcoded frame graph
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

		// Prepare frame graph
		Vadon::Render::FrameGraphInfo frame_graph_info;
		frame_graph_info.name = "MainWindow";

		{
			// Draw to the main window
			frame_graph_info.targets.emplace_back("main_window");
		}

		{
			Vadon::Render::RenderPass& clear_pass = frame_graph_info.passes.emplace_back();
			clear_pass.name = "Clear";

			clear_pass.targets.emplace_back("main_window", "main_window_cleared");

			Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			const Vadon::Render::RTVHandle main_window_target = rt_system.get_window_target(m_render_window);

			clear_pass.execution = [main_window_target, &rt_system]()
				{
					// Clear target and set it
					rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.6f, 1.0f));
					rt_system.set_target(main_window_target, Vadon::Render::DSVHandle());
				};
		}

		{
			Vadon::Render::RenderPass& canvas_pass = frame_graph_info.passes.emplace_back();
			canvas_pass.name = "Canvas";

			canvas_pass.targets.emplace_back("main_window_cleared", "main_window_canvas");

			canvas_pass.execution = [this]()
				{
					// Update camera and viewport
					// FIXME: do this only when it actually changes!
					VadonApp::Platform::PlatformInterface& platform_interface = m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
					const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_drawable_size(m_game_core.get_platform_interface().get_main_window());

					Vadon::Render::Canvas::RenderContext& render_context = get_canvas_context();
					render_context.viewports.back().render_viewport.dimensions.size = window_size;

					// NOTE: here this works trivially because our RT is also the back buffer
					// We just sync up camera projection and viewport
					// If we switch to separate RT and back buffer:
					// - Can use scissor on the RT to cut off parts that won't be visible (probably overkill)
					// - More important: calculate visible portion, use fullscreen copy shader to copy that part to back buffer
					render_context.camera.view_rectangle.size = window_size;

					Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
					canvas_system.render(render_context);
				};
		}

		{
			VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

			Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

			dev_gui_pass.name = "DevGUI";

			dev_gui_pass.targets.emplace_back("main_window_canvas", "main_window_dev_gui");

			dev_gui_pass.execution = [this, &dev_gui_system]()
				{
					if (m_game_core.get_ui_system().is_dev_gui_enabled() == false)
					{
						return;
					}

					dev_gui_system.render();
				};
		}

		Vadon::Render::FrameSystem& frame_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::FrameSystem>();
		m_frame_graph = frame_system.create_graph(frame_graph_info);
		
		if (m_frame_graph.is_valid() == false)
		{
			return false;
		}

		return true;
	}

	bool RenderSystem::init_canvas_context()
	{
		// Initialize canvas context
		Render& common_render = m_game_core.get_core().get_render();
		m_canvas_context = common_render.create_canvas_context();

		// Scene not displayed before, set up new context
		Vadon::Render::Canvas::RenderContext& render_context = common_render.get_context(m_canvas_context);

		// Set up viewport based on main window
		{
			Vadon::Render::RenderTargetSystem& rt_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();

			Vadon::Render::Canvas::Viewport canvas_viewport;
			canvas_viewport.render_target = rt_system.get_window_target(m_render_window);

			render_context.viewports.push_back(canvas_viewport);
		}

		return true;
	}

	void RenderSystem::pre_update()
	{
		// TODO: revise this
		// Allow other systems to register a callback (or use some other mechanism) to delegate
		// filling the contents of the Canvas Item
		// Once the Canvas Item is ready, we can use this callback
		if (m_deferred_init_queue.empty() == false)
		{
			for (Vadon::ECS::EntityHandle current_entity : m_deferred_init_queue)
			{
				init_entity(current_entity);
			}

			m_deferred_init_queue.clear();
		}
	}
	
	void RenderSystem::update()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

		// Execute frame graph
		Vadon::Render::FrameSystem& frame_system = engine_core.get_system<Vadon::Render::FrameSystem>();
		frame_system.execute_graph(m_frame_graph);

		// Present to the main window
		Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
		rt_system.update_window(m_render_window);
	}

	void RenderSystem::init_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		m_game_core.get_core().get_render().update_entity(ecs_world, entity, m_canvas_context);
	}

	void RenderSystem::remove_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		m_game_core.get_core().get_render().remove_entity(ecs_world, entity);
	}
}