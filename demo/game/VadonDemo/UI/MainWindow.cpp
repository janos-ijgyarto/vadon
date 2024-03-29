#include <VadonDemo/UI/MainWindow.hpp>

#include <VadonDemo/Core/GameCore.hpp>
#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Node.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Core/Object/ObjectSystem.hpp>
#include <Vadon/Core/Task/TaskSystem.hpp>

#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>

#include <Vadon/Render/Utilities/Projection.hpp>

#include <Vadon/Utilities/Container/Concurrent/TripleBuffer.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <Vadon/Utilities/Math/Matrix.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>

#include <deque>
#include <format>
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

		constexpr const char* c_vertex_shader_source =
R"(cbuffer Constants : register(b0)
{       
	struct 
	{
		float4x4 projection;
	} Constants;
}

struct VS_INPUT
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
	output.pos = mul(input.pos, Constants.projection);
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

		Render::Shader m_test_shader;
		Vadon::Render::BufferHandle m_vertex_buffer;
		size_t m_current_vertex_count = 0;
		Vadon::Render::PipelineState m_pipeline_state;
		Vadon::Render::BufferHandle m_constant_buffer;

		MainWindowDevGUI m_dev_gui;

		std::vector<Model::Node2D*> m_rendered_nodes;
		Vadon::Render::DrawCommand m_draw_command;

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
			m_rendered_nodes.clear();

			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

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
						engine_core.get_logger().error("Cannot open scene file!\n");
						return;
					}

					Vadon::Utilities::JSON scene_data_json;
					try
					{
						scene_data_json = Vadon::Utilities::JSON::parse(scene_file);
					}
					catch (const Vadon::Utilities::JSON::exception& exception)
					{
						engine_core.get_logger().error(std::string("Error parsing scene file: ") + exception.what() + "\n");
						return;
					}

					scene_file.close();

					Vadon::Utilities::JSONReader scene_data_reader(scene_data_json);
					if (scene_system.load_scene(scene_handle, scene_data_reader) == false)
					{
						engine_core.get_logger().error("Error loading scene file!\n");
						return;
					}

					Vadon::Scene::Node* scene_node = scene_system.instantiate_scene(scene_handle);
					if (scene_node == nullptr)
					{
						engine_core.get_logger().error("Error instantiating scene!\n");
						return;
					}

					scene_system.get_root().add_child(scene_node);
				}
			}
			
			std::deque<Vadon::Scene::Node*> node_queue;
			node_queue.push_front(&scene_system.get_root());

			Vadon::Core::ObjectSystem& object_system = engine_core.get_system<Vadon::Core::ObjectSystem>();
			while (node_queue.empty() == false)
			{
				Vadon::Scene::Node* current_node = node_queue.front();
				Model::Node2D* node_2d = object_system.get_object_as<Model::Node2D>(*current_node);

				if (node_2d != nullptr)
				{
					m_rendered_nodes.push_back(node_2d);
				}

				const Vadon::Scene::NodeList& current_node_children = current_node->get_children();
				node_queue.insert(node_queue.end(), current_node_children.begin(), current_node_children.end());

				node_queue.pop_front();
			}
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
					position_element.format = Vadon::Render::GraphicsAPIDataFormat::R32G32B32A32_FLOAT;
					position_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
					position_element.name = "POSITION";
				}

				{
					Vadon::Render::VertexLayoutElement& color_element = vertex_layout_info.emplace_back();
					color_element.format = Vadon::Render::GraphicsAPIDataFormat::R32G32B32A32_FLOAT;
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

			// Create constant buffer
			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();
			{
				Vadon::Render::BufferInfo cbuffer_info;
				cbuffer_info.type = Vadon::Render::BufferType::CONSTANT;
				cbuffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
				cbuffer_info.element_size = sizeof(Vadon::Utilities::Matrix4);
				cbuffer_info.capacity = 1;

				const Vadon::Utilities::Vector2& window_size = main_window_info.window.size;

				const Vadon::Utilities::Matrix4 projection_matrix = Vadon::Render::create_directx_orthographic_projection_matrix(-window_size.x / 2, window_size.x / 2, -window_size.y / 2, window_size.y / 2, -1.0f, 1.0f, Vadon::Render::CoordinateSystem::RIGHT_HAND);

				Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
				m_constant_buffer = buffer_system.create_buffer(cbuffer_info, &projection_matrix);
			}

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
				const Vadon::Render::RenderTargetHandle main_window_target = rt_system.get_window_target(main_window_info.render_handle);

				clear_pass.execution = [main_window_target, &rt_system]()
				{
					// Clear target and set it
					rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.6f, 1.0f));
					rt_system.set_target(main_window_target, Vadon::Render::DepthStencilHandle());
				};
			}

			{
				Vadon::Render::GraphicsAPI& graphics_api = engine_core.get_system<Vadon::Render::GraphicsAPI>();

				Vadon::Render::RenderPass& test_pass = frame_graph_info.passes.emplace_back();
				test_pass.name = "Triangles";

				test_pass.targets.emplace_back("main_window_cleared", "main_window_triangles");

				test_pass.execution = [&platform_interface, &graphics_api, &shader_system, this]()
				{
					if (m_draw_command.vertices.count == 0)
					{
						// Nothing to draw
						return;
					}

					VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
					Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

					Vadon::Render::PipelineSystem& pipeline_system = engine_core.get_system<Vadon::Render::PipelineSystem>();
					pipeline_system.set_primitive_topology(Vadon::Render::PrimitiveTopology::TRIANGLE_LIST);

					// Reset pipeline state
					pipeline_system.apply_blend_state(m_pipeline_state.blend_update);
					pipeline_system.apply_depth_stencil_state(m_pipeline_state.depth_stencil_update);
					pipeline_system.apply_rasterizer_state(m_pipeline_state.rasterizer_state);

					// Apply shaders
					shader_system.apply_shader(m_test_shader.vertex_shader);
					shader_system.apply_shader(m_test_shader.pixel_shader);

					const VadonApp::Platform::RenderWindowInfo main_window_info = platform_interface.get_window_info();

					Vadon::Render::Viewport viewport;
					viewport.dimensions.size = main_window_info.window.size;

					Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
					rt_system.apply_viewport(viewport);

					shader_system.set_vertex_layout(m_test_shader.vertex_layout);

					// Set buffers
					Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
					buffer_system.set_constant_buffer(Vadon::Render::ShaderType::VERTEX, m_constant_buffer, 0);
					buffer_system.set_vertex_buffer(m_vertex_buffer, 0);

					graphics_api.draw(m_draw_command);
				};
			}

			{
				VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

				dev_gui_pass.name = "DevGUI";

				dev_gui_pass.targets.emplace_back("main_window_triangles", "main_window_dev_gui");

				dev_gui_pass.execution = [this, &dev_gui_system, &shader_system]()
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

		void update_vertex_data(const VertexList& vertices)
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();

			if (vertices.size() > m_current_vertex_count)
			{
				if (m_vertex_buffer.is_valid() == true)
				{
					// Clean up previous buffer
					buffer_system.remove_buffer(m_vertex_buffer);
				}

				Vadon::Render::BufferInfo vertex_buffer_info;
				vertex_buffer_info.type = Vadon::Render::BufferType::VERTEX;
				vertex_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
				vertex_buffer_info.element_size = sizeof(Vertex);
				vertex_buffer_info.capacity = static_cast<int32_t>(vertices.size());

				m_vertex_buffer = buffer_system.create_buffer(vertex_buffer_info);
				m_current_vertex_count = vertices.size();
			}

			// Buffer the vertices
			Vadon::Render::BufferWriteData vertex_buffer_write_data{ .range = { 0, static_cast<int32_t>(vertices.size()) }, .data = vertices.data() };
			buffer_system.buffer_data(m_vertex_buffer, vertex_buffer_write_data);
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
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

			TimePoint current_time = Clock::now();
			const float delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();

			m_last_time_point = current_time;

			// Update the nodes
			// FIXME: should be doing it from the model thread, then sync with render thread!
			engine_core.get_system<Vadon::Scene::SceneSystem>().update(delta_time);

			// Generate the render data
			// FIXME: move this to the shared library so the editor can use it too!
			// FIXME2: implement generalized basic 2D rendering!
			VertexList vertices;

			for (Model::Node2D* render_node : m_rendered_nodes)
			{
				const Vadon::Utilities::Vector2 global_pos = render_node->get_global_position();
				const float scale = render_node->get_scale();
				const Vadon::Utilities::Vector4 color = render_node->get_color();
				{
					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x, global_pos.y + scale, 0, 1), color);
					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x + scale, global_pos.y, 0, 1), color);
					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x - scale, global_pos.y, 0, 1), color);

					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x - scale, global_pos.y, 0, 1), color);
					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x + scale, global_pos.y, 0, 1), color);
					vertices.emplace_back(Vadon::Utilities::Vector4(global_pos.x, global_pos.y - scale, 0, 1), color);
				}
			}

			update_vertex_data(vertices);

			m_draw_command.vertices.offset = 0;
			m_draw_command.vertices.count = static_cast<int32_t>(vertices.size());
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