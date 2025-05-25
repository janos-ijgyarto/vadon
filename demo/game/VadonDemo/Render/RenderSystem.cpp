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

#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

namespace
{
	// TODO: implement base shaders for general-purpose fullscreen effects!
	constexpr const char* c_copy_shader_hlsl =
R"(struct VS_OUTPUT
{
    float4 screen_position : SV_POSITION;
    float2 uv : UV;
};

VS_OUTPUT copy_vs(uint vertex_id : SV_VertexID)
{
	VS_OUTPUT output;

    // Generate triangle that fills screen using vertex ID
    output.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
    
    float2 screen_position = (output.uv * float2(2, -2)) + float2(-1, 1);
    output.screen_position = float4(screen_position, 0, 1);

	return output;
}

sampler main_sampler : register(s0);
Texture2D main_texture : register(t0);

#define PS_INPUT VS_OUTPUT

float4 copy_ps(PS_INPUT input) : SV_Target
{
	return main_texture.Sample(main_sampler, input.uv);
}
)";
}

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

	void RenderSystem::init_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
		if (canvas_component == nullptr)
		{
			return;
		}

		if (canvas_component->canvas_item.is_valid() == true)
		{
			// Canvas item already initialized
			// FIXME: check if context is correctly set?
			return;
		}

		m_game_core.get_core().get_render().init_entity(ecs_world, entity, m_canvas_context);
	}

	Vadon::Utilities::Vector2i RenderSystem::map_to_game_viewport(const Vadon::Utilities::Vector2i& position) const
	{
		const Core::GlobalConfiguration& global_config = m_game_core.get_core().get_global_config();
		return ((Vadon::Utilities::Vector2(position) - m_game_viewport.dimensions.position) / m_game_viewport.dimensions.size) * global_config.viewport_size;
	}

	RenderSystem::RenderSystem(Core::GameCore& game_core)
		: m_game_core(game_core)
		, m_aspect_ratio(0.0f)
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

		Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
		{
			Vadon::Render::ShaderInfo copy_shader_vs_info;
			copy_shader_vs_info.source = c_copy_shader_hlsl;
			copy_shader_vs_info.entrypoint = "copy_vs";
			copy_shader_vs_info.name = "CopyVShader";
			copy_shader_vs_info.type = Vadon::Render::ShaderType::VERTEX;

			m_copy_vshader = shader_system.create_shader(copy_shader_vs_info);
			VADON_ASSERT(m_copy_vshader.is_valid(), "Failed to create vertex shader!");
			if (m_copy_vshader.is_valid() == false)
			{
				return false;
			}
		}

		{
			Vadon::Render::ShaderInfo copy_shader_ps_info;
			copy_shader_ps_info.source = c_copy_shader_hlsl;
			copy_shader_ps_info.entrypoint = "copy_ps";
			copy_shader_ps_info.name = "CopyPShader";
			copy_shader_ps_info.type = Vadon::Render::ShaderType::PIXEL;

			m_copy_pshader = shader_system.create_shader(copy_shader_ps_info);
			VADON_ASSERT(m_copy_pshader.is_valid(), "Failed to create pixel shader!");
			if (m_copy_pshader.is_valid() == false)
			{
				return false;
			}
		}

		// Create texture sampler
		// (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
		{
			Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();

			Vadon::Render::TextureSamplerInfo sampler_info;
			sampler_info.filter = Vadon::Render::TextureFilter::MIN_MAG_MIP_LINEAR;
			sampler_info.address_u = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.address_v = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.address_w = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.mip_lod_bias = 0.0f;
			sampler_info.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;
			sampler_info.min_lod = 0.0f;
			sampler_info.max_lod = 0.0f;

			m_copy_sampler = texture_system.create_sampler(sampler_info);
		}

		if (init_frame_graph() == false)
		{
			return false;
		}

		m_game_core.get_core().add_entity_event_callback(
			[this](Vadon::ECS::World& /*ecs_world*/, const VadonDemo::Core::EntityEvent& event)
			{
				switch (event.type)
				{
				case VadonDemo::Core::EntityEventType::ADDED:
					init_entity(event.entity);
					break;
				case VadonDemo::Core::EntityEventType::REMOVED:
					remove_entity(event.entity);
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

		Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
		const Vadon::Render::RTVHandle main_window_target = rt_system.get_window_back_buffer_view(m_render_window);
		{
			Vadon::Render::RenderPass& clear_pass = frame_graph_info.passes.emplace_back();
			clear_pass.name = "Clear";

			clear_pass.targets.emplace_back("main_window", "main_window_cleared");

			const Vadon::Render::RTVHandle& game_target = m_game_rtv;

			clear_pass.execution = [main_window_target, &game_target, &rt_system]()
				{
					// Clear game and main window targets
					rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.0f, 0.0f));
					rt_system.clear_target(game_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.0f, 0.0f));

					// Set game target
					rt_system.set_target(game_target, Vadon::Render::DSVHandle());
				};
		}

		{
			Vadon::Render::RenderPass& canvas_pass = frame_graph_info.passes.emplace_back();
			canvas_pass.name = "Canvas";

			canvas_pass.targets.emplace_back("main_window_cleared", "game_canvas");

			canvas_pass.execution = [this]()
				{
					Vadon::Render::Canvas::RenderContext& render_context = get_canvas_context();

					Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
					canvas_system.render(render_context);
				};
		}

		{
			Vadon::Render::RenderPass& canvas_copy_pass = frame_graph_info.passes.emplace_back();

			canvas_copy_pass.name = "CanvasCopy";

			canvas_copy_pass.targets.emplace_back("game_canvas", "game_to_main_window");

			Vadon::Render::GraphicsAPI& graphics_api = engine_core.get_system<Vadon::Render::GraphicsAPI>();
			Vadon::Render::ResourceSystem& resource_system = engine_core.get_system<Vadon::Render::ResourceSystem>();
			Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
			Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();
			canvas_copy_pass.execution = [this, &graphics_api, &resource_system, &rt_system, &shader_system, &texture_system, main_window_target]()
				{
					shader_system.apply_shader(m_copy_vshader);
					shader_system.apply_shader(m_copy_pshader);

					rt_system.set_target(main_window_target, Vadon::Render::DSVHandle());

					resource_system.apply_shader_resource(Vadon::Render::ShaderType::PIXEL, m_game_rt_srv, 0);
					texture_system.set_sampler(Vadon::Render::ShaderType::PIXEL, m_copy_sampler, 0);

					rt_system.apply_viewport(m_game_viewport);

					graphics_api.draw(Vadon::Render::DrawCommand{
						.type = Vadon::Render::DrawCommandType::DEFAULT,
						.vertices = { 0, 3 }
						}
					);

					// Unset the SRV (otherwise D3D will complain)
					resource_system.apply_shader_resource(Vadon::Render::ShaderType::PIXEL, Vadon::Render::SRVHandle(), 0);
				};
		}

		{
			VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

			Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

			dev_gui_pass.name = "DevGUI";

			dev_gui_pass.targets.emplace_back("game_to_main_window", "main_window_dev_gui");

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

	bool RenderSystem::init_viewport()
	{
		// Initialize canvas context
		Render& common_render = m_game_core.get_core().get_render();
		m_canvas_context = common_render.create_canvas_context();

		Vadon::Render::Canvas::RenderContext& render_context = common_render.get_context(m_canvas_context);
		const Core::GlobalConfiguration& global_config = m_game_core.get_core().get_global_config();
		VADON_ASSERT(Vadon::Utilities::all(Vadon::Utilities::greaterThan(global_config.viewport_size, Vadon::Utilities::Vector2_Zero)), "Invalid viewport dimensions!");

		// Create RT for game contents
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			
			{
				Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();

				Vadon::Render::TextureInfo rt_texture_info{
					.dimensions = { global_config.viewport_size.x, global_config.viewport_size.y, 0 },
					.mip_levels = 1,
					.array_size = 1,
					.format = Vadon::Render::GraphicsAPIDataFormat::B8G8R8A8_UNORM,
					.sample_info{.count = 1, .quality = 0 },
					.usage = Vadon::Render::ResourceUsage::DEFAULT,
					.flags = Vadon::Render::TextureFlags::SHADER_RESOURCE | Vadon::Render::TextureFlags::RENDER_TARGET,
					.access_flags = Vadon::Render::ResourceCPUAccessFlags::NONE
				};
				m_game_rt_texture = texture_system.create_texture(rt_texture_info);

				Vadon::Render::TextureSRVInfo rt_srv_info{
					.type = Vadon::Render::TextureSRVType::TEXTURE_2D,
					.format = Vadon::Render::GraphicsAPIDataFormat::B8G8R8A8_UNORM,
					.most_detailed_mip = 0,
					.mip_levels = ~0u,
				};

				m_game_rt_srv = texture_system.create_shader_resource_view(m_game_rt_texture, rt_srv_info);
			}

			{
				Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
				Vadon::Render::RenderTargetViewInfo rtv_info = {
					.format = Vadon::Render::GraphicsAPIDataFormat::B8G8R8A8_UNORM,
					.type = Vadon::Render::RenderTargetViewType::TEXTURE_2D,
					.type_info = { .mip_slice = 0 }
				};
					 
				m_game_rtv = rt_system.create_render_target_view(m_game_rt_texture, rtv_info);
			}

			Vadon::Render::Canvas::Viewport canvas_viewport;
			canvas_viewport.render_target = m_game_rtv;

			canvas_viewport.render_viewport.dimensions.size = global_config.viewport_size;

			render_context.viewports.push_back(canvas_viewport);
		}

		render_context.camera.view_rectangle.size = global_config.viewport_size;

		m_aspect_ratio = global_config.viewport_size.x / global_config.viewport_size.y;

		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::WindowHandle main_window = m_game_core.get_platform_interface().get_main_window();
			const Vadon::Utilities::Vector2i window_drawable_size = engine_app.get_system<VadonApp::Platform::PlatformInterface>().get_window_drawable_size(main_window);

			update_viewport(window_drawable_size);
		}

		return true;
	}

	void RenderSystem::update_viewport(const Vadon::Utilities::Vector2i& window_size)
	{
		Vadon::Utilities::Vector2i clamped_size = Vadon::Utilities::max(window_size, Vadon::Utilities::Vector2i{ 1, 1 });

		if (window_size.x > window_size.y)
		{
			m_game_viewport.dimensions.size = { window_size.y * m_aspect_ratio, window_size.y };
			m_game_viewport.dimensions.position = { (window_size.x - m_game_viewport.dimensions.size.x) / 2, 0 };
		}
		else
		{
			m_game_viewport.dimensions.size = { window_size.x, window_size.x * (1.0f / m_aspect_ratio) };
			m_game_viewport.dimensions.position = { 0, (window_size.y - m_game_viewport.dimensions.size.y) / 2 };
		}
	}
	
	void RenderSystem::update()
	{
		process_platform_events();

		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

		// Execute frame graph
		Vadon::Render::FrameSystem& frame_system = engine_core.get_system<Vadon::Render::FrameSystem>();
		frame_system.execute_graph(m_frame_graph);

		// Present to the main window
		Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
		rt_system.update_window(Vadon::Render::WindowUpdateInfo{ .window = m_render_window });
	}

	void RenderSystem::process_platform_events()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

		// Handle window move & resize (affects rendering so it has to happen at the appropriate time)
		for (const VadonApp::Platform::PlatformEvent& current_event : platform_interface.poll_events())
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
					VadonApp::Platform::WindowHandle main_window = m_game_core.get_platform_interface().get_main_window();
					const Vadon::Utilities::Vector2i drawable_size = engine_app.get_system<VadonApp::Platform::PlatformInterface>().get_window_drawable_size(main_window);

					// Resize the window render target
					Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
					rt_system.resize_window(m_render_window, drawable_size);

					// Update the game viewport
					update_viewport(drawable_size);
				}
				break;
				}
			}
			break;
			}
		}
	}

	void RenderSystem::remove_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		m_game_core.get_core().get_render().remove_entity(ecs_world, entity);
	}
}