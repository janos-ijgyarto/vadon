#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/UI/UISystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/Database.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>

namespace
{
	constexpr float c_sim_timestep = 1.0f / 30.0f;
	constexpr float c_frame_limit = 0.25f;

	// FIXME: reorganize all of this, move model code out of UI!
	class GameResourceDatabase : public Vadon::Scene::ResourceDatabase
	{
	public:
		GameResourceDatabase(VadonDemo::Core::GameCore& game_core)
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

		VadonDemo::Core::GameCore& m_game_core;
		std::unordered_map<Vadon::Scene::ResourceID, Vadon::Core::FileSystemPath> m_resource_file_lookup;
	};
}

namespace VadonDemo::Model
{
	struct GameModel::Internal
	{
		Core::GameCore& m_game_core;

		std::unique_ptr<Model> m_model;
		bool m_model_updated = false;

		Vadon::Core::Project m_project_info;
		Vadon::Core::RootDirectoryHandle m_root_directory;

		GameResourceDatabase m_resource_db;

		float m_model_accumulator = 0.0f;
		int m_model_frame_count = 0;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
			, m_resource_db(game_core)
		{
		}

		bool initialize()
		{
			m_model = std::make_unique<Model>(m_game_core.get_engine_app().get_engine_core());

			if (m_model->initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool init_simulation()
		{
			if (load_project() == false)
			{
				return false;
			}

			if (init_scene() == false)
			{
				return false;
			}

			Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
			if (m_model->init_simulation(ecs_world) == false)
			{
				// TODO: error?
				return false;
			}

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
			if (m_resource_db.initialize(m_root_directory) == false)
			{
				return false;
			}
			engine_core.get_system<Vadon::Scene::ResourceSystem>().register_database(m_resource_db);

			// Everything loaded successfully
			return true;
		}

		bool init_scene()
		{
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

			return true;
		}

		void update()
		{
			// Reset update flag
			m_model_updated = false;

			const float delta_time = std::min(m_game_core.get_delta_time(), c_frame_limit);

			// Model update logic inspired by the "Fix Your Timestep!" blog by Gaffer on Games: https://gafferongames.com/post/fix_your_timestep/
			float model_accumulator = m_model_accumulator;
			model_accumulator += delta_time;

			while (model_accumulator >= c_sim_timestep)
			{
				m_model_updated = true;

				update_player_input();

				m_model->update(m_game_core.get_ecs_world(), c_sim_timestep);

				model_accumulator -= c_sim_timestep;				
				++m_model_frame_count;
			}

			m_model_accumulator = model_accumulator;
		}

		void update_player_input()
		{
			// Update movement
			VadonDemo::Model::PlayerInput player_input;

			const Platform::PlatformInterface::InputValues input_values = m_game_core.get_platform_interface().get_input_values();

			UI::UISystem& ui_system = m_game_core.get_ui_system();
			VadonApp::UI::Developer::GUISystem& dev_gui = m_game_core.get_engine_app().get_system<VadonApp::UI::Developer::GUISystem>();
			if ((ui_system.is_dev_gui_enabled() == false) || (Vadon::Utilities::to_bool(dev_gui.get_io_flags() & VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE) == false))
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

			auto player_query = m_game_core.get_ecs_world().get_component_manager().run_component_query<VadonDemo::Model::Player&>();
			auto player_it = player_query.get_iterator();
			if (player_it.is_valid() == true)
			{
				auto player_components = player_it.get_tuple();

				VadonDemo::Model::Player& player_component = std::get<VadonDemo::Model::Player&>(player_components);
				player_component.input = player_input;
			}
		}
	};

	GameModel::~GameModel() = default;

	float GameModel::get_sim_timestep() const
	{
		return c_sim_timestep;
	}

	float GameModel::get_accumulator() const
	{
		return m_internal->m_model_accumulator;
	}

	int GameModel::get_frame_count() const
	{
		return m_internal->m_model_frame_count;
	}

	bool GameModel::is_updated() const
	{
		return m_internal->m_model_updated;
	}

	GameModel::GameModel(Core::GameCore& core, Vadon::Core::EngineEnvironment& environment)
		: m_internal(std::make_unique<Internal>(core))
	{
		Model::init_engine_environment(environment);
	}

	bool GameModel::initialize()
	{
		return m_internal->initialize();
	}

	bool GameModel::init_simulation()
	{
		return m_internal->init_simulation();
	}

	void GameModel::update()
	{
		m_internal->update();
	}
}