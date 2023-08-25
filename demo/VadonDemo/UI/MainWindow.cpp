#include <VadonDemo/UI/MainWindow.hpp>

#include <VadonDemo/Core/GameCore.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Task/TaskSystem.hpp>

#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Utilities/Container/Concurrent/TripleBuffer.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>

#include <format>

namespace VadonDemo::UI
{
	namespace
	{
		constexpr const char* c_vertex_shader_source =
R"(struct VS_INPUT
{
	float4 pos : POSITION;
	float4 col : COLOR0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = input.pos;
	output.col = input.col;
	return output;
})";

		constexpr const char* c_pixel_shader_source =
R"(struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

float4 main(PS_INPUT input) : SV_Target
{
	return input.col; 
})";

		struct Vertex
		{
			Vadon::Utilities::Vector4 position;
			Vadon::Utilities::Vector4 color;
		};

		using VertexList = std::vector<Vertex>;

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
				for(size_t current_list_item_index = 0; current_list_item_index < 10; ++current_list_item_index)
				{
					list_box.items.emplace_back("Item_" + std::to_string(current_list_item_index));
				}
			}
		};
	}

	struct MainWindow::Internal
	{
		Core::GameCore& m_game_core;

		bool m_dev_gui_enabled = false;

		Vadon::Utilities::TripleBuffer<int32_t> m_dev_gui_buffer;
		std::array<VadonApp::Platform::PlatformEventList, 2> m_dev_gui_event_buffer;
		VadonApp::Platform::PlatformEventList* m_dev_gui_event_write_buffer;
		VadonApp::Platform::PlatformEventList* m_dev_gui_event_read_buffer;
		std::mutex m_dev_gui_event_mutex;

		Render::Shader m_test_shader;
		Vadon::Render::BufferHandle m_vertex_buffer;

		MainWindowDevGUI m_dev_gui;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
			, m_dev_gui_buffer({0, 1, 2})
		{
			m_dev_gui_event_write_buffer = &m_dev_gui_event_buffer[0];
			m_dev_gui_event_read_buffer = &m_dev_gui_event_buffer[1];
		}

		bool initialize()
		{
			init_renderer();
			init_dev_gui();

			return true;
		}

		void init_renderer()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			// Create shaders
			Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();

			{
				Vadon::Render::ShaderInfo vertex_shader_info;
				vertex_shader_info.source = c_vertex_shader_source;
				vertex_shader_info.entrypoint = "main";
				vertex_shader_info.type = Vadon::Render::ShaderType::VERTEX;

				m_test_shader.vertex_shader = shader_system.create_shader(vertex_shader_info);
				assert(m_test_shader.vertex_shader.is_valid());

				// Create vertex layout
				Vadon::Render::VertexLayoutInfo vertex_layout_info;

				{
					Vadon::Render::VertexLayoutElement& position_element = vertex_layout_info.emplace_back();
					position_element.format = Vadon::Render::GraphicsAPIDataFormat::FLOAT4_32;
					position_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
					position_element.name = "POSITION";
				}

				{
					Vadon::Render::VertexLayoutElement& color_element = vertex_layout_info.emplace_back();
					color_element.format = Vadon::Render::GraphicsAPIDataFormat::FLOAT4_32;
					color_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
					color_element.name = "COLOR";
				}

				m_test_shader.vertex_layout = shader_system.create_vertex_layout(m_test_shader.vertex_shader, vertex_layout_info);
				assert(m_test_shader.vertex_layout.is_valid());
			}

			{
				Vadon::Render::ShaderInfo pixel_shader_info;
				pixel_shader_info.source = c_pixel_shader_source;
				pixel_shader_info.entrypoint = "main";
				pixel_shader_info.type = Vadon::Render::ShaderType::PIXEL;

				m_test_shader.pixel_shader = shader_system.create_shader(pixel_shader_info);
				assert(m_test_shader.pixel_shader.is_valid());
			}

			// Prepare vertex buffer
			Vadon::Render::BufferInfo vertex_buffer_info;
			vertex_buffer_info.binding = Vadon::Render::BufferBinding::VERTEX;
			vertex_buffer_info.usage = Vadon::Render::BufferUsage::DYNAMIC;
			vertex_buffer_info.element_size = sizeof(Vertex);
			vertex_buffer_info.capacity = 6;

			Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
			m_vertex_buffer = buffer_system.create_buffer(vertex_buffer_info);

			// Prepare frame graph
			Vadon::Render::FrameGraphInfo frame_graph_info;
			frame_graph_info.name = "MainWindow";

			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			{
				// Draw to the main window
				const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

				Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
				const Vadon::Render::RenderTargetHandle main_window_target = rt_system.get_window_target(main_window_info.render_handle);

				frame_graph_info.targets.emplace_back("main_window", main_window_target);
			}

			{
				Vadon::Render::RenderPass& clear_pass = frame_graph_info.passes.emplace_back();
				clear_pass.name = "Clear";

				clear_pass.targets.emplace_back("main_window", "main_window_cleared");

				const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

				Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
				const Vadon::Render::RenderTargetHandle main_window_target = rt_system.get_window_target(main_window_info.render_handle);

				clear_pass.render_function = [main_window_target, &rt_system]()
				{
					rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.6f, 1.0f));
				};
			}

			{
				Vadon::Render::GraphicsAPI& graphics_api = engine_core.get_system<Vadon::Render::GraphicsAPI>();

				Vadon::Render::RenderPass& test_pass = frame_graph_info.passes.emplace_back();
				test_pass.name = "Triangles";

				test_pass.targets.emplace_back("main_window_cleared", "main_window_triangles");

				test_pass.shaders.push_back(m_test_shader.vertex_shader);
				test_pass.shaders.push_back(m_test_shader.pixel_shader);

				test_pass.render_function = [&platform_interface, &graphics_api, this]()
				{
					VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
					Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

					Vadon::Render::PipelineSystem& pipeline_system = engine_core.get_system<Vadon::Render::PipelineSystem>();
					pipeline_system.set_primitive_topology(Vadon::Render::PrimitiveTopology::TRIANGLE_LIST);

					const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

					Vadon::Render::Viewport viewport;
					viewport.dimensions.size = main_window_info.window.size;

					Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
					rt_system.apply_viewport(viewport);

					Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
					shader_system.set_vertex_layout(m_test_shader.vertex_layout);

					Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
					buffer_system.set_vertex_buffer(m_vertex_buffer, 0);

					const Vadon::Render::DrawCommand draw_command{ .vertices = {.offset = 0, .count = 6 } };
					graphics_api.draw(draw_command);
				};
			}

			{
				VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

				dev_gui_pass.name = "DevGUI";

				dev_gui_pass.targets.emplace_back("main_window_triangles", "main_window_dev_gui");

				dev_gui_pass.shaders = dev_gui_system.get_shaders();
				dev_gui_pass.pipeline_state = dev_gui_system.get_pipeline_state();

				dev_gui_pass.render_function = [this, &dev_gui_system]()
				{
					const int32_t* read_frame_index = m_dev_gui_buffer.get_read_buffer();
					if (read_frame_index)
					{
						// Read frame ready, swap with the ready frame
						dev_gui_system.swap_frame(*read_frame_index, 3);
					}
					// Always render the ready frame
					dev_gui_system.render_frame(3);
				};

				dev_gui_pass.validator_function = [this]()
				{
					return m_dev_gui_enabled;
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
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			// TODO: add something to make this more dynamic?
			static VertexList test_vertices = {
				{ { -0.5f, 0.5f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { -0.25f, 0.5f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { -0.5f, 0.25f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
				{ { -0.1f, -0.1f, 1.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
				{ { 0.0f, 0.1f, 1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
				{ { 0.1f, -0.1f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
			};

			// Buffer the vertices
			Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
			buffer_system.buffer_data(m_vertex_buffer, Vadon::Utilities::DataRange{ 0, 6 }, test_vertices.data(), true);
		}

		void test_tasks()
		{
			// First create a task group to encompass everything
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Core::TaskSystem& task_system = engine_core.get_system<Vadon::Core::TaskSystem>();

			Vadon::Core::TaskGroup test_task_group = task_system.create_task_group("Vadondemo_task_test");

			// Add logging to know where we are
			test_task_group->add_start_subtask(
				[&task_system]()
				{
					task_system.log("Starting test task group...\n");
				}
			);

			test_task_group->add_end_subtask(
				[&task_system]()
				{
					task_system.log("Test task group finished!\n");
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
					task_system.log("Spawning additional nodes...\n");

					// Test spawning tasks from within a task
					Vadon::Core::TaskNode post_process_task = task_system.create_task_node("Vadondemo_task_test_post_process");

					// Make sure post-process happens before group is done
					test_task_group->add_end_dependency(post_process_task);

					post_process_task->add_subtask(
						[pi_array, &task_system]()
						{
							task_system.log("Running post-process task...\n");

							float result = 0.0f;
							for (float pi_part : *pi_array)
							{
								result += pi_part;
							}

							task_system.log(std::format("Final result: {}\n", result));
						}
					);

					// Add a node that will store the subtasks
					Vadon::Core::TaskNode subtask_node = task_system.create_task_node("Vadondemo_task_test_subtasks");
					subtask_node->add_dependent(post_process_task);

					// Test multiple subtasks
					for (size_t current_task_index = 0; current_task_index < pi_array->size(); ++current_task_index)
					{
						subtask_node->add_subtask(
							[pi_array, current_task_index, &task_system]()
							{
								task_system.log(std::format("Running task node #{}\n", current_task_index));

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

								task_system.log(std::format("Task node #{} finished, result is {}\n", current_task_index, result));
							}
						);
					}

					// Parent task now done
					task_system.log("Root task finished!\n");

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