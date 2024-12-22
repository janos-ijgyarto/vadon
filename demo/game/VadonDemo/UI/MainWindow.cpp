#include <VadonDemo/UI/MainWindow.hpp>

#include <VadonDemo/Core/GameCore.hpp>
#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Core/Project/Project.hpp>
#include <Vadon/Core/Task/TaskSystem.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Scene/Resource/Database.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Container/Concurrent/TripleBuffer.hpp>
#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>

namespace VadonDemo::UI
{
	namespace
	{
		constexpr float c_sim_timestep = 1.0f / 30.0f;
		constexpr float c_frame_limit = 0.25f;

		struct MainWindowDevGUI
		{
			VadonApp::UI::Developer::Window window;
			std::string output_label;
			std::string output_text;

			VadonApp::UI::Developer::InputInt input_int;
			VadonApp::UI::Developer::InputInt2 input_int2;
			VadonApp::UI::Developer::InputFloat input_float;
			VadonApp::UI::Developer::InputFloat2 input_float2;
			VadonApp::UI::Developer::InputText input_text;
			VadonApp::UI::Developer::Button button;

			VadonApp::UI::Developer::Checkbox checkbox;

			VadonApp::UI::Developer::SliderInt slider_int;
			VadonApp::UI::Developer::SliderInt2 slider_int2;
			VadonApp::UI::Developer::SliderFloat slider_float;
			VadonApp::UI::Developer::SliderFloat2 slider_float2;
			float slider_float_cache = 0.0f;

			VadonApp::UI::Developer::InputFloat3 color_picker;

			VadonApp::UI::Developer::Button console_button;

			VadonApp::UI::Developer::Button task_test_button;

			VadonApp::UI::Developer::ListBox list_box;
			VadonApp::UI::Developer::ComboBox combo_box;

			VadonApp::UI::Developer::Table table;
			std::vector<int32_t> table_data;

			void initialize()
			{
				window.title = "Demo dev GUI window";
				window.flags = VadonApp::UI::Developer::WindowFlags::ENABLE_CLOSE;

				output_label = "Your text here:";
				output_text = "Sample Text";

				input_int.label = "Input Int";
				input_int.input = 0;

				input_int2.label = "Input Int2";
				input_int2.input = { 0,0 };

				input_float.label = "Input Float";
				input_float.input = 0.0f;

				input_float2.label = "Input Float2";
				input_float2.input = { 0.0f, 0.0f };

				input_text.label = "Text to add to label";
				input_text.multiline = true;

				button.label = "Put text in label";

				checkbox.label = "Checkbox";
				checkbox.checked = true;

				slider_int.label = "Slider Int";
				slider_int.value = 0;
				slider_int.min = -10;
				slider_int.max = 10;

				slider_int2.label = "Slider Int2";
				slider_int2.value = { 0, 0 };
				slider_int2.min = -10;
				slider_int2.max = 10;

				slider_float.label = "Slider float";
				slider_float.value = 1.0f;
				slider_float.min = 0.0f;
				slider_float.max = 10.0f;
				slider_float.format = "%.4f";

				slider_float2.label = "Slider float2";
				slider_float2.value = { 1.0f, 1.0f };
				slider_float2.min = 0.0f;
				slider_float2.max = 10.0f;
				slider_float2.format = "%.4f";

				color_picker.label = "Color picker";
				color_picker.input = { 0,0,0 };

				console_button.label = "Show console";

				task_test_button.label = "Test task system";

				list_box.label = "List Box";
				combo_box.label = "Combo Box";
				for(size_t current_list_item_index = 0; current_list_item_index < 10; ++current_list_item_index)
				{
					list_box.items.emplace_back("List_Item_" + std::to_string(current_list_item_index));
					combo_box.items.emplace_back("Combo_Item_" + std::to_string(current_list_item_index));
				}

				constexpr int32_t table_size = 100;
				table.label = "Table";
				table.column_count = table_size;

				table_data.reserve(table_size * table_size);
				for (int32_t current_y = 0; current_y < table_size; ++current_y)
				{
					for (int32_t current_x = 0; current_x < table_size; ++current_x)
					{
						table_data.push_back(current_x + (current_y * table_size));
					}
				}
			}
		};

		struct CameraData
		{
			Vadon::Utilities::Vector2 camera_position = Vadon::Utilities::Vector2_Zero;
			float camera_zoom = 1.0f;
		};

		// FIXME: reorganize all of this, move model code out of UI!
		class GameResourceDatabase : public Vadon::Scene::ResourceDatabase
		{
		public:
			GameResourceDatabase(Core::GameCore& game_core)
				: m_game_core(game_core)
			{
			}

			bool initialize(Vadon::Core::RootDirectoryHandle project_root)
			{
				// Import all resources in the project
				bool all_valid = true;

				// FIXME: make this modular!
				// Scene system should load scene files!
				std::string extensions_string = ".vdsc,.vdrc";

				Vadon::Core::FileSystem& file_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Core::FileSystem>();

				const std::vector<Vadon::Core::FileSystemPath> resource_files = file_system.get_files_of_type(Vadon::Core::FileSystemPath{ .root_directory = project_root }, extensions_string, true);
				for (const Vadon::Core::FileSystemPath& current_file_path : resource_files)
				{
					all_valid &= import_resource(current_file_path).is_valid();
				}

				return all_valid;
			}

			bool save_resource(Vadon::Scene::ResourceSystem& /*resource_system*/, Vadon::Scene::ResourceHandle /*resource_handle*/) override
			{
				// We won't be saving resources 
				return false;
			}

			Vadon::Scene::ResourceHandle load_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceID resource_id) override
			{
				auto resource_path_it = m_resource_file_lookup.find(resource_id);
				if (resource_path_it == m_resource_file_lookup.end())
				{
					return Vadon::Scene::ResourceHandle();
				}

				Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
				Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
				Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
				if (file_system.load_file(resource_path_it->second, resource_file_buffer) == false)
				{
					resource_system.log_error("Game resource database: failed to load resource file!\n");
					return Vadon::Scene::ResourceHandle();
				}

				// FIXME: support binary file serialization!
				// Solution: have file system create the appropriate serializer!
				Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

				if (serializer_instance->initialize() == false)
				{
					resource_system.log_error("Game resource database: failed to initialize serializer while loading resource!\n");
					return Vadon::Scene::ResourceHandle();
				}

				Vadon::Scene::ResourceHandle loaded_resource_handle = resource_system.load_resource(*serializer_instance);
				if (loaded_resource_handle.is_valid() == false)
				{
					resource_system.log_error("Game resource database: failed to load resource data!\n");
					return loaded_resource_handle;
				}

				if (serializer_instance->finalize() == false)
				{
					resource_system.log_error("Game resource database: failed to finalize serializer after loading resource!\n");
				}

				return loaded_resource_handle;
			}

		private:
			// FIXME: this forces us to load all resources twice!
			// Need to create a "cache" that has all this metadata
			// Can be created by the editor when the project is exported
			Vadon::Scene::ResourceID import_resource(const Vadon::Core::FileSystemPath& path)
			{
				// TODO: deduplicate parts shared with loading a resource!
				Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
				Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
				Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
				if (file_system.load_file(path, resource_file_buffer) == false)
				{
					Vadon::Core::Logger::log_error("Game resource database: failed to load resource file!\n");
					return Vadon::Scene::ResourceID();
				}

				// FIXME: support binary file serialization!
				// Solution: have file system create the appropriate serializer!
				Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

				if (serializer_instance->initialize() == false)
				{
					Vadon::Core::Logger::log_error("Game resource database: failed to initialize serializer while loading resource!\n");
					return Vadon::Scene::ResourceID();
				}

				Vadon::Scene::ResourceInfo imported_resource_info;
				Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
				if (resource_system.load_resource_info(*serializer_instance, imported_resource_info) == false)
				{
					Vadon::Core::Logger::log_error("Game resource database: failed to loading resource!\n");
					return Vadon::Scene::ResourceID();
				}
								
				m_resource_file_lookup[imported_resource_info.id] = path;

				return imported_resource_info.id;
			}

			Core::GameCore& m_game_core;
			std::unordered_map<Vadon::Scene::ResourceID, Vadon::Core::FileSystemPath> m_resource_file_lookup;
		};
	}

	struct MainWindow::Internal
	{
		Core::GameCore& m_game_core;
				
		bool m_dev_gui_enabled = false;

		Vadon::Utilities::TripleBuffer<int32_t> m_dev_gui_render_buffer;

		MainWindowDevGUI m_dev_gui;

		// FIXME: move these to subsystems!
		Vadon::Core::Project m_project_info;
		GameResourceDatabase m_resource_db;
		Vadon::Core::RootDirectoryHandle m_root_directory;
		Vadon::Render::Canvas::RenderContext m_canvas_context;

		Vadon::Utilities::TripleBuffer<Vadon::Utilities::PacketQueue> m_render_data_buffer;
		Vadon::Utilities::TripleBuffer<CameraData> m_camera_data_buffer;
		Vadon::Render::Canvas::Camera m_model_camera;

		float m_model_accumulator = 0.0f;
		int m_model_frame_count = 0;
		int m_view_frame_count = 0;
		int m_render_frame_count = 0;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
			, m_dev_gui_render_buffer({0, 1, 2})
			, m_resource_db(game_core)
		{
		}

		bool initialize()
		{
			if (init_scene() == false)
			{
				return false;
			}
			init_renderer();
			init_dev_gui();

			return true;
		}

		bool load_project()
		{
			// First validate the path
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			// Check command line arg, load startup project if requested
			constexpr const char c_startup_project_arg[] = "project";
			if (m_game_core.has_command_line_arg(c_startup_project_arg) == false)
			{
				// TODO: error message!
				return false;
			}
			
			std::filesystem::path fs_root_path(m_game_core.get_command_line_arg(c_startup_project_arg));
			if (Vadon::Core::Project::is_valid_project_path(fs_root_path.string()) == false)
			{
				constexpr const char c_invalid_path_error[] = "Game demo: invalid project path!\n";

				// Assume we gave a path to the folder with the project file
				if (std::filesystem::is_directory(fs_root_path) == true)
				{
					fs_root_path /= Vadon::Core::Project::c_project_file_name;
					if (Vadon::Core::Project::is_valid_project_path(fs_root_path.string()) == false)
					{
						Vadon::Core::Logger::log_error(c_invalid_path_error);
						return false;
					}
				}
				else
				{
					Vadon::Core::Logger::log_error(c_invalid_path_error);
					return false;
				}
			}

			const std::string project_file_path = fs_root_path.string();
			if (m_project_info.load_project_file(engine_core, project_file_path) == false)
			{
				Vadon::Core::Logger::log_error(std::format("Invalid project file at \"{}\"!\n", project_file_path));
				return false;
			}

			if (m_project_info.startup_scene.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			{
				Vadon::Core::RootDirectoryInfo project_dir_info;
				project_dir_info.path = m_project_info.root_path;

				// Add project root directory
				Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
				m_root_directory = file_system.add_root_directory(project_dir_info);
				if (m_root_directory.is_valid() == false)
				{
					Vadon::Core::Logger::log_error("Failed to register project root directory!\n");
					return false;
				}
			}

			// Import all resources in the project
			// FIXME: do this automatically?
			if(m_resource_db.initialize(m_root_directory) == false)
			{
				return false;
			}
			engine_core.get_system<Vadon::Scene::ResourceSystem>().register_database(m_resource_db);

			// Everything loaded successfully
			return true;
		}

		bool init_scene()
		{
			if (load_project() == false)
			{
				return false;
			}

			using Logger = Vadon::Core::Logger;

			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();
			Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
			
			{
				Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();
				const Vadon::Scene::SceneHandle main_scene_handle = scene_system.load_scene(m_project_info.startup_scene);
				if (main_scene_handle.is_valid() == false)
				{
					// Something went wrong
					return false;
				}

				Vadon::ECS::EntityHandle root_entity_handle = scene_system.instantiate_scene(main_scene_handle, ecs_world);
				if (root_entity_handle.is_valid() == false)
				{
					// Something went wrong
					return false;
				}
			}

			if (m_game_core.get_model().init_simulation(ecs_world) == false)
			{
				// TODO: error?
				return false;
			}

			{
				VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
				const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

				m_model_camera.view_rectangle.size = main_window_info.window.size;
				m_canvas_context.camera = m_model_camera;

				{
					Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();

					Vadon::Render::Canvas::Viewport canvas_viewport;
					canvas_viewport.render_target = rt_system.get_window_target(main_window_info.render_handle);
					canvas_viewport.render_viewport.dimensions.size = main_window_info.window.size;

					m_canvas_context.viewports.push_back(canvas_viewport);

					m_canvas_context.layers.push_back(m_game_core.get_model().get_canvas_layer());
				}
			}

			return true;
		}

		void init_renderer()
		{
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

				VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
				const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

				Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
				const Vadon::Render::RenderTargetHandle main_window_target = rt_system.get_window_target(main_window_info.render_handle);

				clear_pass.execution = [main_window_target, &rt_system]()
				{
					// Clear target and set it
					rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.6f, 1.0f));
					rt_system.set_target(main_window_target, Vadon::Render::DepthStencilHandle());
				};
			}

			{
				Vadon::Render::RenderPass& canvas_pass = frame_graph_info.passes.emplace_back();
				canvas_pass.name = "Canvas";

				canvas_pass.targets.emplace_back("main_window_cleared", "main_window_canvas");

				canvas_pass.execution = [this]()
					{
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
					if(m_dev_gui_enabled == false)
					{
						return;
					}

					const int32_t* read_frame_index = m_dev_gui_render_buffer.get_read_buffer();
					if (read_frame_index)
					{
						// Read frame ready, swap with the ready frame
						dev_gui_system.swap_frame(*read_frame_index, 3);
					}

					// Always render the ready frame
					dev_gui_system.render_frame(3);
				};
			}

			// Add to the frame graph
			m_game_core.get_render_system().set_frame_graph(frame_graph_info);
		}

		void init_dev_gui()
		{
			m_dev_gui.initialize();
		}

		Vadon::Core::TaskGroup update()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			Vadon::Core::TaskSystem& task_system = engine_core.get_system<Vadon::Core::TaskSystem>();

			Vadon::Core::TaskGroup main_window_task_group = task_system.create_task_group("Vadondemo_main_window_model");
			Vadon::Core::TaskNode model_update_node = main_window_task_group->create_start_dependent("Vadondemo_model_update");

			const float delta_time = std::min(m_game_core.get_delta_time(), c_frame_limit);

			model_update_node->add_subtask(
				[this, delta_time]()
				{
					// Model update logic inspired by the "Fix Your Timestep!" blog by Gaffer on Games: https://gafferongames.com/post/fix_your_timestep/
					float model_accumulator = m_model_accumulator;
					model_accumulator += delta_time;
					bool model_updated = false;

					while (model_accumulator >= c_sim_timestep)
					{
						m_game_core.get_model().update_simulation(m_game_core.get_ecs_world(), c_sim_timestep);
						model_accumulator -= c_sim_timestep;
						model_updated = true;
						++m_model_frame_count;
					}

					m_model_accumulator = model_accumulator;

					if(model_updated == true)
					{
						Vadon::Utilities::PacketQueue* render_queue = m_render_data_buffer.get_write_buffer();
						m_game_core.get_model().update_view_async(m_game_core.get_ecs_world(), *render_queue);

						m_render_data_buffer.set_write_complete();
					}
				}
			);

			Vadon::Core::TaskNode view_update_node = model_update_node->create_dependent("Vadondemo_view_update");
			view_update_node->add_subtask(
				[this, &engine_app, delta_time]()
				{
					++m_view_frame_count;

					// Update movement
					VadonDemo::Model::PlayerInput player_input;

					const Platform::PlatformInterface::InputValues input_values = m_game_core.get_platform_interface().get_input_values();

					VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
					if ((m_dev_gui_enabled == false) || (Vadon::Utilities::to_bool(dev_gui.get_io_flags() & VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE) == false))
					{
						if (input_values.move_left == true)
						{
							player_input.move_dir.x = -1.0f;
						}
						else if (input_values.move_right == true)
						{
							player_input.move_dir.x = 1.0f;
						}

						if (input_values.move_up == true)
						{
							player_input.move_dir.y = 1.0f;
						}
						else if (input_values.move_down == true)
						{
							player_input.move_dir.y = -1.0f;
						}

						player_input.fire = input_values.fire;
					}

					m_game_core.get_model().set_player_input(m_game_core.get_ecs_world(), player_input);

					auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Transform2D&, VadonDemo::Model::Player&>();
					for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
					{
						auto player_components = player_it.get_tuple();
						m_model_camera.view_rectangle.position = std::get<VadonDemo::Model::Transform2D&>(player_components).position;
					}
					// TODO: camera zoom?

					{
						CameraData* camera_data = m_camera_data_buffer.get_write_buffer();
						camera_data->camera_position = m_model_camera.view_rectangle.position;
						camera_data->camera_zoom = m_model_camera.zoom;

						m_camera_data_buffer.set_write_complete();
					}
				}
			);

			// Render dev GUI (if enabled)
			// FIXME: extract to function?
			if(m_dev_gui_enabled)
			{
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
				if (!m_dev_gui.window.open)
				{
					// Window was closed, disable dev GUI
					m_dev_gui_enabled = false;
					return main_window_task_group;
				}

				// Add a node to start the dev GUI frame				
				Vadon::Core::TaskNode gui_start_node = view_update_node->create_dependent("Vadondemo_dev_gui_start");
				gui_start_node->add_subtask(
					[this, &dev_gui]()
					{
						dev_gui.start_frame();
					}
				);

				// Wrap the console in a task node
				VadonApp::UI::Console& app_console = engine_app.get_system<VadonApp::UI::UISystem>().get_console();
				Vadon::Core::TaskNode console_gui_node = gui_start_node->create_dependent("Vadondemo_console_gui");
				console_gui_node->add_subtask(
					[&app_console]()
					{
						app_console.render();
					}
				);

				// Run main window GUI after console
				Vadon::Core::TaskNode main_window_node = console_gui_node->create_dependent("Vadondemo_main_window_gui");
				main_window_node->add_subtask(
					[this, &engine_app, &dev_gui]()
					{
						if (dev_gui.begin_window(m_dev_gui.window))
						{
							dev_gui.add_text(std::format("Model frames: {}", m_model_frame_count));
							dev_gui.add_text(std::format("View frames: {}", m_view_frame_count));
							dev_gui.add_text(std::format("Render frames: {}", m_render_frame_count));

							if (dev_gui.push_tree_node("Numeric Inputs"))
							{
								dev_gui.draw_input_int(m_dev_gui.input_int);
								dev_gui.draw_input_int2(m_dev_gui.input_int2);
								dev_gui.draw_input_float(m_dev_gui.input_float);
								dev_gui.draw_input_float2(m_dev_gui.input_float2);

								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Text"))
							{
								dev_gui.add_text(m_dev_gui.output_label);
								dev_gui.add_text(m_dev_gui.output_text);

								dev_gui.draw_input_text(m_dev_gui.input_text);

								if (dev_gui.draw_button(m_dev_gui.button))
								{
									m_dev_gui.output_text = m_dev_gui.input_text.input;
								}

								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Buttons"))
							{
								dev_gui.draw_checkbox(m_dev_gui.checkbox);
								if (m_dev_gui.checkbox.checked)
								{
									dev_gui.add_text("Checkbox checked!");
								}

								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Sliders"))
							{
								dev_gui.draw_slider_int(m_dev_gui.slider_int);
								dev_gui.draw_slider_int2(m_dev_gui.slider_int2);
								if (dev_gui.draw_slider_float(m_dev_gui.slider_float))
								{
									m_dev_gui.slider_float_cache = m_dev_gui.slider_float.value;
								}
								dev_gui.draw_slider_float2(m_dev_gui.slider_float2);

								dev_gui.add_text(std::format("Last slider value: {:.4f}", m_dev_gui.slider_float_cache));

								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Color"))
							{
								dev_gui.draw_color3_picker(m_dev_gui.color_picker);
								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("List Box"))
							{
								dev_gui.draw_list_box(m_dev_gui.list_box);
								dev_gui.add_text(std::format("Selected list item: index = {}, text = \"{}\"", m_dev_gui.list_box.selected_item, m_dev_gui.list_box.items[m_dev_gui.list_box.selected_item]));
								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Combo Box"))
							{
								dev_gui.draw_combo_box(m_dev_gui.combo_box);
								dev_gui.add_text(std::format("Selected combo item: index = {}, text = \"{}\"", m_dev_gui.combo_box.selected_item, m_dev_gui.combo_box.items[m_dev_gui.combo_box.selected_item]));
								dev_gui.pop_tree_node();
							}

							if (dev_gui.push_tree_node("Table"))
							{
								if (dev_gui.begin_table(m_dev_gui.table) == true)
								{
									for (int32_t current_value : m_dev_gui.table_data)
									{
										dev_gui.next_table_column();
										dev_gui.add_text(std::to_string(current_value));
									}
									dev_gui.end_table();
								}
								dev_gui.pop_tree_node();
							}

							if (dev_gui.draw_button(m_dev_gui.console_button))
							{
								engine_app.get_system<VadonApp::UI::UISystem>().get_console().show();
							}

							if (dev_gui.draw_button(m_dev_gui.task_test_button))
							{
								test_tasks();
							}
						}
						dev_gui.end_window();
					}
				);

				// End dev GUI after main window
				Vadon::Core::TaskNode gui_end_node = main_window_node->create_dependent("Vadondemo_dev_gui_end");
				gui_end_node->add_subtask(
					[this, &dev_gui]()
					{
						dev_gui.end_frame();

						const int32_t* write_frame_index = m_dev_gui_render_buffer.get_write_buffer();
						dev_gui.cache_frame(*write_frame_index);
						m_dev_gui_render_buffer.set_write_complete();
					}
				);

				main_window_task_group->add_end_dependency(gui_end_node);
			}
			
			return main_window_task_group;
		}

		void render()
		{
			// Update using data from model
			Vadon::Utilities::PacketQueue* render_packets = m_render_data_buffer.get_read_buffer();
			if (render_packets != nullptr)
			{
				m_game_core.get_model().render_async(*render_packets);
			}

			CameraData* camera_data = m_camera_data_buffer.get_read_buffer();
			if (camera_data != nullptr)
			{
				m_canvas_context.camera.view_rectangle.position = camera_data->camera_position;
				m_canvas_context.camera.zoom = camera_data->camera_zoom;
			}

			++m_render_frame_count;
			m_game_core.get_model().lerp_view_state(m_model_accumulator / c_sim_timestep);
		}

		void test_tasks()
		{
			using Logger = Vadon::Core::Logger;

			// First create a task group to encompass everything
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Core::TaskSystem& task_system = engine_core.get_system<Vadon::Core::TaskSystem>();

			Vadon::Core::TaskGroup test_task_group = task_system.create_task_group("Vadondemo_task_test");

			// Add logging to know where we are
			test_task_group->add_start_subtask(
				[]()
				{
					Logger::log_message("Starting test task group...\n");
				}
			);

			test_task_group->add_end_subtask(
				[]()
				{
					Logger::log_message("Test task group finished!\n");
				}
			);

			Vadon::Core::TaskNode spawner_node = task_system.create_task_node("Vadondemo_task_test_spawner");
			test_task_group->add_start_dependent(spawner_node);

			// As a simple test case, use the infinite series for calculating pi
			// Create an array to test a "shared resource"
			std::shared_ptr<std::array<float, 4>> pi_array = std::make_shared<std::array<float, 4>>();
			pi_array->fill(0.0f);

			spawner_node->add_subtask(
				[&task_system, test_task_group, pi_array]()
				{
					// Another log
					Logger::log_message("Spawning additional nodes...\n");

					// Test spawning tasks from within a task
					Vadon::Core::TaskNode post_process_task = task_system.create_task_node("Vadondemo_task_test_post_process");

					// Make sure post-process happens before group is done
					test_task_group->add_end_dependency(post_process_task);

					post_process_task->add_subtask(
						[pi_array]()
						{
							Logger::log_message("Running post-process task...\n");

							float result = 0.0f;
							for (float pi_part : *pi_array)
							{
								result += pi_part;
							}

							Logger::log_message(std::format("Final result: {}\n", result));
						}
					);

					// Add a node that will store the subtasks
					Vadon::Core::TaskNode subtask_node = task_system.create_task_node("Vadondemo_task_test_subtasks");
					subtask_node->add_dependent(post_process_task);

					// Test multiple subtasks
					for (size_t current_task_index = 0; current_task_index < pi_array->size(); ++current_task_index)
					{
						subtask_node->add_subtask(
							[pi_array, current_task_index]()
							{
								Logger::log_message(std::format("Running task node #{}\n", current_task_index));

								float& result = (*pi_array)[current_task_index];
								int denominator = 1;

								constexpr size_t c_sum_length = 1000;

								for (size_t current_sum_index = 0; current_sum_index < c_sum_length; ++current_sum_index)
								{
									const float current_part = (1.0f / static_cast<float>(denominator));
									if ((current_sum_index % 2) == 0)
									{
										result += current_part;
									}
									else
									{
										result -= current_part;
									}
									denominator += 2;
								}

								Logger::log_message(std::format("Task node #{} finished, result is {}\n", current_task_index, result));
							}
						);
					}

					// Parent task now done
					Logger::log_message("Root task finished!\n");

					// Kick generated node into queue
					subtask_node->update();
				}
			);				

			// Kick the group into the queue
			test_task_group->update();
		}

		void show_dev_gui()
		{
			m_dev_gui_enabled = true;
			m_dev_gui.window.open = true;
		}
	};

	MainWindow::~MainWindow() = default;

	MainWindow::MainWindow(Core::GameCore& game_core)
		: m_internal(std::make_unique<Internal>(game_core))
	{

	}

	bool MainWindow::initialize()
	{
		return m_internal->initialize();
	}

	Vadon::Core::TaskGroup MainWindow::update()
	{
		return m_internal->update();
	}

	void MainWindow::render()
	{
		m_internal->render();
	}

	void MainWindow::show_dev_gui()
	{
		m_internal->show_dev_gui();
	}
}