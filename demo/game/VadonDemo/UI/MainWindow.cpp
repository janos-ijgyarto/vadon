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

#include <Vadon/Core/Task/TaskSystem.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Container/Concurrent/TripleBuffer.hpp>

#include <deque>
#include <filesystem>
#include <fstream>

namespace VadonDemo::UI
{
	namespace
	{
		using Clock = std::chrono::steady_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		using Duration = std::chrono::duration<float>;

		TimePoint last_frame_time = Clock::now();

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
	}

	struct MainWindow::Internal
	{
		Core::GameCore& m_game_core;

		TimePoint m_last_time_point;

		bool m_dev_gui_enabled = false;

		Vadon::Utilities::TripleBuffer<int32_t> m_dev_gui_buffer;
		std::array<VadonApp::Platform::PlatformEventList, 2> m_dev_gui_event_buffer;
		VadonApp::Platform::PlatformEventList* m_dev_gui_event_write_buffer;
		VadonApp::Platform::PlatformEventList* m_dev_gui_event_read_buffer;
		std::mutex m_dev_gui_event_mutex;

		MainWindowDevGUI m_dev_gui;

		Vadon::Render::Canvas::RenderContext m_canvas_context;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
			, m_dev_gui_buffer({0, 1, 2})
		{
			m_dev_gui_event_write_buffer = &m_dev_gui_event_buffer[0];
			m_dev_gui_event_read_buffer = &m_dev_gui_event_buffer[1];
		}

		bool initialize()
		{
			init_scene();
			init_renderer();
			init_dev_gui();

			m_last_time_point = Clock::now();

			return true;
		}

		void init_scene()
		{
			using Logger = Vadon::Core::Logger;

			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();
			Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();

			// Register component events
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			component_manager.register_event_callback<VadonDemo::Model::CanvasItem>(
				[this](const Vadon::ECS::ComponentEvent& event)
				{
					m_game_core.get_model().component_event(m_game_core.get_ecs_world(), event);
				}
			);
			component_manager.register_event_callback<VadonDemo::Model::Celestial>(
				[this, &ecs_world](const Vadon::ECS::ComponentEvent& event)
				{
					m_game_core.get_model().component_event(m_game_core.get_ecs_world(), event);
				}
			);

			Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();

			// FIXME: load using project file!
			{
				Vadon::Scene::SceneInfo scene_info;
				scene_info.name = "TestScene";

				Vadon::Scene::SceneHandle scene_handle = scene_system.create_scene(scene_info);

				const std::string scene_file_path = scene_info.name + ".vdsc";
				if (std::filesystem::exists(scene_file_path) == true)
				{
					std::ifstream scene_file(scene_file_path);
					if (scene_file.is_open() == false)
					{
						Logger::log_error("Cannot open scene file!\n");
						return;
					}

					std::vector<std::byte> scene_data_buffer;
					{
						scene_file.seekg(0, std::ios::end);
						std::streampos file_size = scene_file.tellg();
						scene_file.seekg(0, std::ios::beg);

						scene_data_buffer.resize(file_size);

						scene_file.read(reinterpret_cast<char*>(scene_data_buffer.data()), file_size);
					}

					scene_file.close();

					Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(scene_data_buffer, Vadon::Utilities::Serializer::Type::JSON, true);
					if (serializer->initialize() == false)
					{
						Logger::log_error(serializer->get_last_error() + '\n');
						return;
					}

					if (scene_system.serialize_scene(scene_handle, *serializer) == false)
					{
						return;
					}

					if (serializer->finalize() == false)
					{
						Logger::log_error(serializer->get_last_error() + '\n');
						return;
					}

					Vadon::ECS::EntityHandle root_entity_handle = scene_system.instantiate_scene(scene_handle, ecs_world);
					if (root_entity_handle.is_valid() == false)
					{
						// Something went wrong
						return;
					}
				}
			}

			m_game_core.get_model().init_simulation(ecs_world);

			{
				VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
				const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

				m_canvas_context.camera.view_rectangle.size = main_window_info.window.size;

				{
					Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();

					Vadon::Render::Canvas::Viewport canvas_viewport;
					canvas_viewport.render_target = rt_system.get_window_target(main_window_info.render_handle);
					canvas_viewport.render_viewport.dimensions.size = main_window_info.window.size;

					m_canvas_context.viewports.push_back(canvas_viewport);

					m_canvas_context.layers.push_back(m_game_core.get_model().get_canvas_layer());
				}
			}
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

					const int32_t* read_frame_index = m_dev_gui_buffer.get_read_buffer();
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

			VadonDemo::Platform::PlatformInterface& platform_interface = m_game_core.get_platform_interface();
			platform_interface.register_event_callback(
				[this](const VadonApp::Platform::PlatformEventList& event_list)
				{
					std::lock_guard lock(m_dev_gui_event_mutex);
					VadonApp::Platform::PlatformEventList& write_buffer = *m_dev_gui_event_write_buffer;
					write_buffer.insert(write_buffer.end(), event_list.begin(), event_list.end());
				}
			);
		}

		Vadon::Core::TaskGroup update()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			Vadon::Core::TaskSystem& task_system = engine_core.get_system<Vadon::Core::TaskSystem>();

			Vadon::Core::TaskGroup main_window_task_group = task_system.create_task_group("Vadondemo_main_window");

			// Render dev GUI (if enabled)
			// FIXME: extract to function?
			if(m_dev_gui_enabled)
			{
				if (!m_dev_gui.window.open)
				{
					// Window was closed, disable dev GUI
					m_dev_gui_enabled = false;
					return main_window_task_group;
				}

				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				// Add a node to start the dev GUI frame				
				Vadon::Core::TaskNode gui_start_node = main_window_task_group->create_start_dependent("Vadondemo_dev_gui_start");
				gui_start_node->add_subtask(
					[this, &dev_gui]()
					{
						{
							std::lock_guard lock(m_dev_gui_event_mutex);
							VadonApp::Platform::PlatformEventList& read_buffer = *m_dev_gui_event_read_buffer;
							dev_gui.dispatch_platform_events(read_buffer);
							read_buffer.clear();

							// Swap the read and write buffer pointers
							VadonApp::Platform::PlatformEventList* read_buffer_temp = m_dev_gui_event_read_buffer;
							m_dev_gui_event_read_buffer = m_dev_gui_event_write_buffer;
							m_dev_gui_event_write_buffer = read_buffer_temp;
						}

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

						const int32_t* write_frame_index = m_dev_gui_buffer.get_write_buffer();
						dev_gui.cache_frame(*write_frame_index);
						m_dev_gui_buffer.set_write_complete();
					}
				);

				// Wait for dev GUI before the main window group is done
				main_window_task_group->add_end_dependency(gui_end_node);
			}

			return main_window_task_group;
		}

		void render()
		{
			TimePoint current_time = Clock::now();
			const float delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();

			m_last_time_point = current_time;

			// Update model
			// FIXME: have separate view, sync with model thread!
			m_game_core.get_model().update_simulation(m_game_core.get_ecs_world(), delta_time);
			m_game_core.get_model().update_rendering(m_game_core.get_ecs_world());
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