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

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

namespace VadonDemo::View
{
	struct GameView::Internal
	{
		Core::GameCore& m_game_core;

		std::unique_ptr<View> m_view;
		int m_view_frame_count = 0;

		Vadon::Render::Canvas::RenderContext m_canvas_context;

		std::vector<Vadon::ECS::EntityHandle> m_uninitialized_entities;

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