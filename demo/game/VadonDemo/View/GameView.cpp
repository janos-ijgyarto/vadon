#include <VadonDemo/View/GameView.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/GameModel.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonDemo/UI/UISystem.hpp>

#include <VadonDemo/View/View.hpp>
#include <VadonDemo/View/Component.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <filesystem>

namespace VadonDemo::View
{
	struct GameView::Internal
	{
		Core::GameCore& m_game_core;

		std::unique_ptr<View> m_view;
		int m_view_frame_count = 0;

		Vadon::Render::Canvas::RenderContext m_canvas_context;

		std::vector<Vadon::ECS::EntityHandle> m_uninitialized_entities;

		// FIXME: refactor this, need a proper asset management system!
		struct SpriteTexture
		{
			Vadon::Render::TextureHandle texture;
			Vadon::Render::SRVHandle srv;
		};

		std::unordered_map<std::string, SpriteTexture> m_sprite_textures;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{ }

		bool initialize()
		{
			if (init_renderer() == false)
			{
				return false;
			}

			m_view = std::make_unique<View>(m_game_core.get_engine_app().get_engine_core());
			if (m_view->initialize() == false)
			{
				return false;
			}

			if (init_canvas() == false)
			{
				return false;
			}

			if (m_view->init_visualization(m_game_core.get_ecs_world()) == false)
			{
				// TODO: error?
				return false;
			}

			return true;
		}

		bool init_renderer()
		{
			// TODO: revise this!
			// Render system should allow registering "passes" or "renderers" (TODO: terminology)
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
				const Vadon::Render::RTVHandle main_window_target = rt_system.get_window_target(m_game_core.get_render_system().get_render_window());

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
						m_canvas_context.viewports.back().render_viewport.dimensions.size = window_size;

						// NOTE: here this works trivially because our RT is also the back buffer
						// We just sync up camera projection and viewport
						// If we switch to separate RT and back buffer:
						// - Can use scissor on the RT to cut off parts that won't be visible (probably overkill)
						// - More important: calculate visible portion, use fullscreen copy shader to copy that part to back buffer
						m_canvas_context.camera.view_rectangle.size = window_size;

						Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
						canvas_system.render(m_canvas_context);
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

			// Add to the frame graph
			m_game_core.get_render_system().set_frame_graph(frame_graph_info);

			return true;
		}

		bool init_canvas()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

			m_canvas_context.camera.view_rectangle.size = { 1024, 768 };

			Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();

			Vadon::Render::Canvas::Viewport canvas_viewport;
			canvas_viewport.render_target = rt_system.get_window_target(m_game_core.get_render_system().get_render_window());
			canvas_viewport.render_viewport.dimensions.size = platform_interface.get_window_drawable_size(m_game_core.get_platform_interface().get_main_window());

			m_canvas_context.viewports.push_back(canvas_viewport);

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
			Vadon::Render::Canvas::LayerHandle canvas_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

			m_canvas_context.layers.push_back(canvas_layer);

			// Add the UI layer
			// FIXME: make this more modular!
			{				
				m_canvas_context.layers.push_back(m_game_core.get_ui_system().get_canvas_layer());
			}

			Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			component_manager.register_event_callback<VadonDemo::View::ViewComponent>(
				[this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
				{
					switch (component_event.event_type)
					{
					case Vadon::ECS::ComponentEventType::ADDED:
						m_uninitialized_entities.push_back(component_event.owner);
						break;
					case Vadon::ECS::ComponentEventType::REMOVED:
						remove_view_entity(ecs_world, component_event.owner);
						break;
					}
				}
			);

			return true;
		}

		void update()
		{
			Model::GameModel& game_model = m_game_core.get_model();
			if (game_model.is_updated() == true)
			{
				// Model was updated, so we have to extract the state for the view
				m_view->extract_model_state(m_game_core.get_ecs_world());
			}

			if (m_uninitialized_entities.empty() == false)
			{
				for (Vadon::ECS::EntityHandle current_entity : m_uninitialized_entities)
				{
					init_view_entity(m_game_core.get_ecs_world(), current_entity);
				}
				m_uninitialized_entities.clear();
			}

			// Interpolate the view state based on model accumulator
			m_view->lerp_view_state(m_game_core.get_ecs_world(), game_model.get_accumulator() / game_model.get_sim_timestep());
			
			update_camera();

			// Update frame counter
			++m_view_frame_count;
		}

		void update_camera()
		{
			// Also update camera w.r.t player
			// TODO: camera zoom?
			auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&, VadonDemo::View::ViewComponent&>();
			auto player_it = player_query.get_iterator();
			if (player_it.is_valid() == true)
			{
				auto player_components = player_it.get_tuple();

				Model::GameModel& game_model = m_game_core.get_model();
				const float lerp_factor = game_model.get_accumulator() / game_model.get_sim_timestep();

				const VadonDemo::View::ViewComponent& player_view_component = std::get<VadonDemo::View::ViewComponent&>(player_components);
				m_canvas_context.camera.view_rectangle.position = player_view_component.prev_transform.position * (1.0f - lerp_factor) + player_view_component.current_transform.position * lerp_factor;
			}
		}

		// FIXME: deduplicate between editor and game?
		void init_view_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);
			if (view_component == nullptr)
			{
				return;
			}

			if (view_component->canvas_item.is_valid() == true)
			{
				return;
			}

			// Ensure resource is valid
			if (view_component->resource.is_valid() == true)
			{
				init_view_resource(view_component->resource);
			}

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
			view_component->canvas_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = m_canvas_context.layers.front() });

			// Update view
			// FIXME: could also prompt the view to iterate through all newly created entities
			m_view->update_view_entity_draw_data(ecs_world, entity);
		}

		void remove_view_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);

			if (view_component == nullptr)
			{
				return;
			}

			if (view_component->canvas_item.is_valid() == false)
			{
				return;
			}

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.remove_item(view_component->canvas_item);
			view_component->canvas_item.invalidate();
		}

		void init_view_resource(VadonDemo::View::ViewResourceHandle resource_handle)
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

			if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::Sprite>(), resource_info.type_id))
			{
				// Check if referenced sprite is already loaded
				VadonDemo::View::SpriteResourceHandle sprite_handle = VadonDemo::View::SpriteResourceHandle::from_resource_handle(resource_handle);

				VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

				auto sprite_it = m_sprite_textures.find(sprite_resource->texture_path);
				if (sprite_it == m_sprite_textures.end())
				{
					// Sprite not yet loaded
					update_sprite_resource(sprite_handle);
				}
			}
		}

		void update_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
		{
			load_sprite_resource(sprite_handle);
			m_view->update_view_resource(m_game_core.get_ecs_world(), VadonDemo::View::ViewResourceHandle::from_resource_handle(sprite_handle));
		}

		void load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
		{
			// FIXME: this should not be here, use asset management system to decouple View from any file management!
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
			VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

			if (sprite_resource->texture_path.empty())
			{
				sprite_resource->texture_srv.invalidate();
				return;
			}

			auto sprite_it = m_sprite_textures.find(sprite_resource->texture_path);
			if (sprite_it != m_sprite_textures.end())
			{
				sprite_resource->texture_srv = sprite_it->second.srv;
				return;
			}

			// FIXME: accept other extensions!
			std::filesystem::path texture_fs_path = sprite_resource->texture_path;
			texture_fs_path.replace_extension(".dds");

			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
			Vadon::Core::FileSystemPath file_path{ .root_directory = m_game_core.get_project_root_dir(), .path = texture_fs_path.string()};

			if (file_system.does_file_exist(file_path) == false)
			{
				sprite_resource->texture_srv.invalidate();
				return;
			}

			Vadon::Core::FileSystem::RawFileDataBuffer texture_file_buffer;
			if (file_system.load_file(file_path, texture_file_buffer) == false)
			{
				sprite_resource->texture_srv.invalidate();
				return;
			}

			Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();
			Vadon::Render::TextureHandle texture_handle = texture_system.create_texture_from_memory(texture_file_buffer.size(), texture_file_buffer.data());
			if (texture_handle.is_valid() == false)
			{
				sprite_resource->texture_srv.invalidate();
				return;
			}

			const Vadon::Render::TextureInfo texture_info = texture_system.get_texture_info(texture_handle);

			// Create texture view
			// FIXME: we are guessing the 
			Vadon::Render::TextureSRVInfo texture_srv_info;
			texture_srv_info.format = texture_info.format;
			texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
			texture_srv_info.mip_levels = texture_info.mip_levels;
			texture_srv_info.most_detailed_mip = 0;

			Vadon::Render::SRVHandle srv_handle = texture_system.create_shader_resource_view(texture_handle, texture_srv_info);
			if (srv_handle.is_valid() == false)
			{
				// TODO: also remove texture?
				sprite_resource->texture_srv.invalidate();
				return;
			}

			// Add to lookup
			SpriteTexture new_sprite_texture{ .texture = texture_handle, .srv = srv_handle };
			m_sprite_textures.insert(std::make_pair(sprite_resource->texture_path, new_sprite_texture));

			sprite_resource->texture_srv = srv_handle;
		}
	};

	GameView::~GameView() = default;

	int GameView::get_frame_count() const
	{
		return m_internal->m_view_frame_count;
	}

	GameView::GameView(Core::GameCore& core)
		: m_internal(std::make_unique<Internal>(core))
	{ }

	bool GameView::initialize()
	{
		return m_internal->initialize();
	}

	void GameView::update()
	{
		m_internal->update();
	}
}