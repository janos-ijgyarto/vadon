#include <VadonDemo/UI/MainWindow.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/UI/UISystem.hpp>

#include <VadonDemo/View/GameView.hpp>

#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <format>

namespace
{
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

			list_box.label = "List Box";
			combo_box.label = "Combo Box";
			for (size_t current_list_item_index = 0; current_list_item_index < 10; ++current_list_item_index)
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

namespace VadonDemo::UI
{
	struct MainWindow::Internal
	{
		Core::GameCore& m_game_core;

		// TODO: Dev GUI displays for model, view, etc.
		MainWindowDevGUI m_dev_gui;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{
		}

		bool initialize()
		{
			init_dev_gui();

			return true;
		}

		void init_dev_gui()
		{
			m_dev_gui.initialize();

			// Add callback to dev GUI
			m_game_core.get_ui_system().register_dev_gui_callback(
				[this](VadonApp::UI::Developer::GUISystem& dev_gui)
				{
					draw_dev_gui(dev_gui);
				}
			);
		}

		void update()
		{
			// TODO: anything?
		}

		void draw_dev_gui(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			if (dev_gui.begin_window(m_dev_gui.window))
			{
				{
					static constexpr const char* c_game_state_labels[] = {
						"INIT",
						"LOADING",
						"RUNNING"
					};

					Model::GameModel& game_model = m_game_core.get_model();
					const Model::GameModel::State game_state = game_model.get_state();
					dev_gui.add_text(std::format("Game state: {}", c_game_state_labels[Vadon::Utilities::to_integral(game_state)]));

					if (game_state == Model::GameModel::State::RUNNING)
					{
						static constexpr const char* c_sim_state_labels[] = {
							"INVALID",
							"PLAYING",
							"PAUSED",
							"GAME OVER"
						};

						const Model::GameModel::SimState sim_state = game_model.get_sim_state();
						dev_gui.add_text(std::format("Sim state: {}", c_sim_state_labels[Vadon::Utilities::to_integral(sim_state)]));

						// TODO: display which level we loaded!
						if (sim_state != Model::GameModel::SimState::GAME_OVER)
						{
							dev_gui.add_text(std::format("Player health: {}", game_model.get_player_health()));
						}
					}

					dev_gui.add_text(std::format("Model frames: {}", game_model.get_frame_count()));
					dev_gui.add_text(std::format("View frames: {}", m_game_core.get_view().get_frame_count()));
				}

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
					m_game_core.get_ui_system().show_console();
				}
			}
			dev_gui.end_window();
		}

		void show()
		{
			// FIXME: separate this from the non-dev GUI window?
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

	void MainWindow::update()
	{
		m_internal->update();
	}

	void MainWindow::show()
	{
		m_internal->show();
	}
}