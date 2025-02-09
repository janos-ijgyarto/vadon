#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Component.hpp>
#include <VadonDemo/Model/GameModel.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>
#include <VadonDemo/UI/UISystem.hpp>
#include <VadonDemo/View/GameView.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/Resource/Database.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <chrono>
#include <format>
#include <filesystem>

namespace
{
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

		Vadon::Core::FileSystemPath find_resource_file(Vadon::Scene::ResourceID resource_id) const
		{
			auto resource_file_it = m_resource_file_lookup.find(resource_id);
			if (resource_file_it != m_resource_file_lookup.end())
			{
				return resource_file_it->second;
			}

			return Vadon::Core::FileSystemPath();
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

namespace VadonDemo::Core
{
	namespace
	{
		using Clock = std::chrono::steady_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		using Duration = std::chrono::duration<float>;
	}

	struct GameCore::Internal
	{
		Core m_core;
		GameResourceDatabase m_resource_db;

		Model::GameModel m_game_model;
		Platform::PlatformInterface m_platform_interface;
		Render::RenderSystem m_render_system;
		UI::UISystem m_ui_system;
		View::GameView m_game_view;

		VadonApp::Core::Application::Instance m_engine_app;

		Vadon::ECS::World m_ecs_world;
		Vadon::ECS::EntityHandle m_core_entity;

		// TODO: move to subsystem?
		Vadon::Core::Project m_project_info;
		Vadon::Core::RootDirectoryHandle m_root_directory;

		// FIXME: implement a proper CLI parser!
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_command_line_args;

		TimePoint m_last_time_point;
		float m_delta_time = 0.0f;

		bool m_shutdown_requested = false;

		Internal(GameCore& game_core, Vadon::Core::EngineEnvironment& environment)
			: m_core(environment)
			, m_resource_db(game_core)
			, m_game_model(game_core)
			, m_platform_interface(game_core)
			, m_render_system(game_core)
			, m_ui_system(game_core)
			, m_game_view(game_core)
		{
		}

		bool initialize(int argc, char* argv[])
		{
			// FIXME: implement a proper CLI parser!
			parse_command_line(argc, argv);

			if (init_engine_application(argc, argv) == false)
			{
				Vadon::Core::Logger::log_error("Failed to initialize engine application!\n");
				return false;
			}

			if (!m_platform_interface.initialize())
			{
				return false;
			}

			if (!m_render_system.initialize())
			{
				return false;
			}

			Core::register_types();

			if (load_project() == false)
			{
				return false;
			}

			// Load core entity
			{
				VADON_ASSERT(m_project_info.startup_scene.is_valid() == true, "No startup scene!");
				Vadon::Scene::SceneSystem& scene_system = m_engine_app->get_engine_core().get_system<Vadon::Scene::SceneSystem>();
				Vadon::Scene::SceneHandle startup_scene_handle = scene_system.load_scene(m_project_info.startup_scene);
				m_core_entity = scene_system.instantiate_scene(startup_scene_handle, m_ecs_world);
			}

			if (m_ui_system.initialize() == false)
			{
				return false;
			}

			if (m_game_model.initialize() == false)
			{
				return false;
			}

			if (m_game_view.initialize() == false)
			{
				return false;
			}

			// FIXME: move these commands to the relevant subsystems!
			m_ui_system.register_console_command("pause",
				[this](std::string_view value)
				{
					// TODO: error if param is incorrect?
					if (value.empty() == true)
					{
						return;
					}

					if (value == "1")
					{
						m_game_model.set_paused(true);
					}
					else if (value == "0")
					{
						m_game_model.set_paused(false);
					}
				}
			);

			m_ui_system.register_console_command("quit",
				[this](std::string_view)
				{
					m_game_model.quit_level();
				}
			);

			Vadon::Core::Logger::log_message("Vadon Demo app initialized.\n");

			m_last_time_point = Clock::now();

			return true;
		}

		void parse_command_line(int argc, char* argv[])
		{
			m_program_name = argv[0];

			for (int32_t current_arg_index = 1; current_arg_index < argc; ++current_arg_index)
			{
				parse_command_line_argument(argv[current_arg_index]);
			}
		}

		void parse_command_line_argument(const char* argument_ptr)
		{
			const std::string argument_string(argument_ptr);

			const size_t equals_char_offset = argument_string.find('=');

			// TODO: check that the arg isn't just whitespace

			if (equals_char_offset == std::string::npos)
			{
				// No equals char found, assume it's a command
				m_command_line_args.emplace(argument_string, "");
				return;
			}

			// Emplace the argument name and value
			const std::string arg_name = argument_string.substr(0, equals_char_offset);
			const std::string arg_value = argument_string.substr(equals_char_offset + 1);

			m_command_line_args.emplace(arg_name, arg_value);
		}

		bool init_engine_application(int /*argc*/, char* argv[])
		{
			// Prepare app config
			// TODO: move to a dedicated subsystem!
			VadonApp::Core::Configuration app_configuration;

			// Prepare app config
			{
				app_configuration.app_config.program_name = argv[0];
			}

			// Prepare engine config
			{
				app_configuration.engine_config.core_config.program_name = argv[0];
			}

			// Prepare UI config
			{
				// TODO
			}

			m_engine_app = VadonApp::Core::Application::create_instance();
			return m_engine_app->initialize(app_configuration);
		}

		int execute(int argc, char* argv[])
		{
			if (initialize(argc, argv) == false)
			{
				Vadon::Core::Logger::log_error("Vadon Demo app failed to initialize!\n");
				shutdown();
				return 1;
			}

			while (m_shutdown_requested == false)
			{
				TimePoint current_time = Clock::now();

				m_delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();
				m_last_time_point = current_time;

				// First process platform events
				m_platform_interface.update(); 
				
				// Update model and view
				m_game_model.update();
				m_game_view.update();

				// Update the UI
				m_ui_system.update();

				// Render the results
				m_render_system.update();
			}

			shutdown();
			return 0;
		}

		void request_shutdown()
		{
			m_shutdown_requested = true;
		}

		void shutdown()
		{
			// TODO: shut down game-side systems
			m_engine_app->shutdown();

			Vadon::Core::Logger::log_message("Vadon Demo app successfully shut down.\n");
		}

		bool has_command_line_arg(std::string_view name) const
		{
			auto arg_it = m_command_line_args.find(name.data());
			if (arg_it == m_command_line_args.end())
			{
				return false;
			}

			return true;
		}

		std::string get_command_line_arg(std::string_view name) const
		{
			auto arg_it = m_command_line_args.find(name.data());
			if (arg_it == m_command_line_args.end())
			{
				return std::string();
			}

			return arg_it->second;
		}

		bool load_project()
		{
			// First validate the path
			Vadon::Core::EngineCoreInterface& engine_core = m_engine_app->get_engine_core();

			// Check command line arg, load startup project if requested
			constexpr const char c_startup_project_arg[] = "project";
			if (has_command_line_arg(c_startup_project_arg) == false)
			{
				// TODO: error message!
				return false;
			}

			std::filesystem::path fs_root_path(get_command_line_arg(c_startup_project_arg));
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

			// Add project root directory
			{
				Vadon::Core::RootDirectoryInfo project_dir_info;
				project_dir_info.path = m_project_info.root_path;

				Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
				m_root_directory = file_system.add_root_directory(project_dir_info);
				if (m_root_directory.is_valid() == false)
				{
					Vadon::Core::Logger::log_error("Failed to register project root directory!\n");
					return false;
				}
			}

			if (m_resource_db.initialize(m_root_directory) == false)
			{
				return false;
			}

			// Register the resource DB
			engine_core.get_system<Vadon::Scene::ResourceSystem>().register_database(m_resource_db);

			// Everything loaded successfully
			return true;
		}
	};

	GameCore::GameCore(Vadon::Core::EngineEnvironment& environment)
		: m_internal(std::make_unique<Internal>(*this, environment))
	{
		VadonApp::Core::Application::init_application_environment(environment);
	}

	GameCore::~GameCore() = default;

	int GameCore::execute(int argc, char* argv[])
	{
		return m_internal->execute(argc, argv);
	}

	VadonApp::Core::Application& GameCore::get_engine_app()
	{ 
		return *m_internal->m_engine_app;
	}

	Platform::PlatformInterface& GameCore::get_platform_interface()
	{
		return m_internal->m_platform_interface;
	}

	Render::RenderSystem& GameCore::get_render_system()
	{
		return m_internal->m_render_system;
	}
	
	UI::UISystem& GameCore::get_ui_system()
	{
		return m_internal->m_ui_system;
	}

	float GameCore::get_delta_time() const
	{
		return m_internal->m_delta_time;
	}

	Model::GameModel& GameCore::get_model()
	{
		return m_internal->m_game_model;
	}

	View::GameView& GameCore::get_view()
	{
		return m_internal->m_game_view;
	}

	Vadon::ECS::World& GameCore::get_ecs_world()
	{
		return m_internal->m_ecs_world;
	}

	const CoreComponent& GameCore::get_core_component() const
	{
		CoreComponent* core_component = m_internal->m_ecs_world.get_component_manager().get_component<CoreComponent>(m_internal->m_core_entity);
		VADON_ASSERT(core_component != nullptr, "Cannot find core component!");
		return *core_component;
	}

	const Vadon::Core::Project& GameCore::get_project_info() const
	{
		return m_internal->m_project_info;
	}

	Vadon::Core::RootDirectoryHandle GameCore::get_project_root_dir() const
	{
		return m_internal->m_root_directory;
	}

	bool GameCore::has_command_line_arg(std::string_view name) const
	{
		return m_internal->has_command_line_arg(name);
	}

	std::string GameCore::get_command_line_arg(std::string_view name) const
	{
		return m_internal->get_command_line_arg(name);
	}

	void GameCore::request_shutdown()
	{
		m_internal->request_shutdown();
	}
}