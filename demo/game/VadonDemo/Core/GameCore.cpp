#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Core/Core.hpp>
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

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>
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
	// FIXME: this is a very hacky solution, should instead use "manifests" to map between resource IDs and files
	Vadon::Scene::ResourceID decode_resource_id_from_file(const std::filesystem::path& file_path)
	{
		// Decode resource ID from file name
		const std::string file_stem = file_path.stem().generic_string();
		Vadon::Scene::ResourceID resource_id;
		VADON_ASSERT(file_stem.length() == resource_id.data.size() * 2, "Invalid file name!");

		char temp_chars[] = "00";
		for (size_t i = 0; i < resource_id.data.size(); ++i)
		{
			int value = 0;
			temp_chars[0] = file_stem[i * 2];
			temp_chars[1] = file_stem[(i * 2) + 1];
			std::from_chars(temp_chars, temp_chars + 2, value, 16);
			resource_id.data[i] = static_cast<char>(value);
		}

		return resource_id;
	}

	class GameResourceDatabase : public Vadon::Scene::ResourceDatabase
	{
	public:
		GameResourceDatabase(VadonDemo::Core::GameCore& game_core)
			: m_game_core(game_core)
		{
		}

		bool initialize(std::string_view root_dir, bool is_raw_data)
		{
			m_is_raw = is_raw_data;
			if (is_raw_data)
			{
				return initialize_raw(root_dir);
			}
			else
			{
				return initialize_exported(root_dir);
			}
		}

		bool initialize_raw(std::string_view root_dir)
		{
			Vadon::Core::FileSystem& file_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Core::FileSystem>();
			
			// Create both resource and asset file databases
			{
				Vadon::Core::FileDatabaseInfo resource_db_info;
				resource_db_info.root_path = std::filesystem::path(root_dir).generic_string();
				resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				m_resource_file_db = file_system.create_database(resource_db_info);
			}

			{
				Vadon::Core::FileDatabaseInfo asset_db_info;
				asset_db_info.root_path = std::filesystem::path(root_dir).generic_string();
				asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				m_asset_file_db = file_system.create_database(asset_db_info);
			}

			// Register all resource files under root
			bool all_valid = true;
			for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_dir))
			{
				if (directory_entry.is_regular_file() == false)
				{
					continue;
				}

				const std::filesystem::path current_file_path = directory_entry.path();
				all_valid &= import_raw_resource(current_file_path);
			}

			return true;
		}

		bool import_raw_resource(const std::filesystem::path& file_path)
		{
			const std::string file_extension = file_path.extension().generic_string();
			bool is_asset_file = false;
			if ((file_extension == ".vdrc") || (file_extension == ".vdsc"))
			{
				// TODO: anything?
			}
			else if(file_extension == ".vdimport")
			{
				is_asset_file = true;
			}
			else
			{
				// Not a relevant file, skip
				return true;
			}

			Vadon::Core::RawFileDataBuffer resource_file_buffer;

			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			const std::string file_path_str = file_path.generic_string();
			if (file_system.load_file(file_path_str, resource_file_buffer) == false)
			{
				Vadon::Core::Logger::log_error(std::format("Game resource database: failed to load resource file \"{}\"!\n", file_path_str));
				return false;
			}

			Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

			if (serializer_instance->initialize() == false)
			{
				Vadon::Core::Logger::log_error("Game resource database: failed to initialize serializer while loading resource!\n");
				return false;
			}

			Vadon::Scene::ResourceInfo imported_resource_info;

			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
			if (resource_system.load_resource_info(*serializer_instance, imported_resource_info) == false)
			{
				Vadon::Core::Logger::log_error("Game resource database: failed to loading resource!\n");
				return false;
			}

			if (serializer_instance->finalize() == false)
			{
				Vadon::Core::Logger::log_error("Game resource database: failed to finalize serializer while loading resource!\n");
				return false;
			}

			// TODO: check resource info to make sure we're only importing a resource once?

			Vadon::Core::FileInfo file_info;
			file_info.path = file_system.get_relative_path(m_resource_file_db, file_path_str);

			if (file_system.add_existing_file(m_resource_file_db, imported_resource_info.id, file_info) == false)
			{
				// TODO: log error?
				VADON_ERROR("Cannot add resource file!");
				return false;
			}

			if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::FileResource>(), imported_resource_info.type_id) == true)
			{
				// Resource points to file, make sure we import that file as well
				if (import_raw_asset_file(imported_resource_info.id) == false)
				{
					// TODO: log error?
					VADON_ERROR("Cannot add file referenced by resource!");
					return false;
				}
			}

			return true;
		}

		bool import_raw_asset_file(const Vadon::Scene::ResourceID& file_id)
		{
			// NOTE: this assumes the resource file has already been registered!
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			const Vadon::Core::FileInfo resource_file_info = file_system.get_file_info(m_resource_file_db, file_id);
			const std::filesystem::path resource_file_path = std::filesystem::path(resource_file_info.path).generic_string();

			VADON_ASSERT(resource_file_path.extension() == ".vdimport", "Invalid file type!");

			const std::filesystem::path imported_file_path = (resource_file_path.parent_path() / resource_file_path.stem()).generic_string();

			// Make sure the file actually exists at the designated path
			if (std::filesystem::exists(file_system.get_absolute_path(m_asset_file_db, imported_file_path.generic_string())) == false)
			{
				// TODO: log error!
				return false;
			}

			Vadon::Core::FileInfo imported_file_info;
			imported_file_info.path = imported_file_path.generic_string();

			if (file_system.add_existing_file(m_asset_file_db, file_id, imported_file_info) == false)
			{
				// TODO: log error!
				return false;
			}

			return true;
		}

		bool initialize_exported(std::string_view root_dir)
		{
			Vadon::Core::FileSystem& file_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Core::FileSystem>();

			{
				Vadon::Core::FileDatabaseInfo resource_db_info;
				resource_db_info.root_path = (std::filesystem::path(root_dir) / "resources").generic_string();
				resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				m_resource_file_db = file_system.create_database(resource_db_info);

				// Register all resource files
				for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(resource_db_info.root_path))
				{
					if (directory_entry.is_regular_file() == false)
					{
						continue;
					}

					const std::filesystem::path current_file_path = directory_entry.path();
					if (current_file_path.extension().generic_string() == ".vdbin")
					{
						Vadon::Scene::ResourceID file_id = decode_resource_id_from_file(current_file_path);

						Vadon::Core::FileInfo file_info;
						file_info.path = current_file_path.filename().generic_string();

						if (file_system.add_existing_file(m_resource_file_db, file_id, file_info) == false)
						{
							// TODO: log error?
							return false;
						}
					}
				}
			}

			{
				Vadon::Core::FileDatabaseInfo asset_db_info;
				asset_db_info.root_path = (std::filesystem::path(root_dir) / "assets").generic_string();
				asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				m_asset_file_db = file_system.create_database(asset_db_info);

				for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(asset_db_info.root_path))
				{
					if (directory_entry.is_regular_file() == false)
					{
						continue;
					}

					const std::filesystem::path current_file_path = directory_entry.path();
					if (current_file_path.extension().generic_string() == ".vdbin")
					{
						Vadon::Scene::ResourceID file_id = decode_resource_id_from_file(current_file_path);

						Vadon::Core::FileInfo file_info;
						file_info.path = current_file_path.filename().generic_string();

						if (file_system.add_existing_file(m_asset_file_db, file_id, file_info) == false)
						{
							// TODO: log error?
							return false;
						}
					}
				}
			}

			return true;
		}

		bool save_resource(Vadon::Scene::ResourceSystem& /*resource_system*/, Vadon::Scene::ResourceHandle /*resource_handle*/) override
		{
			// We won't be saving resources 
			return false;
		}

		Vadon::Scene::ResourceHandle load_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceID resource_id) override
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			Vadon::Core::RawFileDataBuffer resource_file_buffer;
			if (file_system.load_file(m_resource_file_db, resource_id, resource_file_buffer) == false)
			{
				resource_system.log_error("Game resource database: failed to load resource file!\n");
				return Vadon::Scene::ResourceHandle();
			}

			// TODO: create separate DBs for binary and text-based resources
			// When a resource is requested, we try both to see if either has the resource
			// Depending on which DB it is, we create the appropriate serializer
			Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, m_is_raw ? Vadon::Utilities::Serializer::Type::JSON : Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::READ);

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

		Vadon::Core::FileInfo get_file_resource_info(Vadon::Scene::ResourceID resource_id) const override
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			return file_system.get_file_info(m_asset_file_db, resource_id);
		}
		
		bool load_file_resource_data(Vadon::Scene::ResourceSystem& /*resource_system*/, Vadon::Scene::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) override
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_game_core.get_engine_app().get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			return file_system.load_file(m_asset_file_db, resource_id, file_data);
		}
	private:
		VadonDemo::Core::GameCore& m_game_core;
		Vadon::Core::FileDatabaseHandle m_resource_file_db;
		Vadon::Core::FileDatabaseHandle m_asset_file_db;
		bool m_is_raw = false;
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
		Vadon::Core::EngineCorePtr m_engine_core;
		std::unique_ptr<Core> m_core;

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
		GlobalConfiguration m_global_config;

		TimePoint m_last_time_point;
		float m_delta_time = 0.0f;

		bool m_shutdown_requested = false;

		Internal(GameCore& game_core, Vadon::Core::EngineEnvironment& environment)
			: m_engine_core(Vadon::Core::create_engine_core())
			, m_resource_db(game_core)
			, m_game_model(game_core)
			, m_platform_interface(game_core)
			, m_render_system(game_core)
			, m_ui_system(game_core)
			, m_game_view(game_core)
		{
			Core::init_environment(environment);
		}

		bool initialize(int argc, char* argv[])
		{
			m_engine_app = VadonApp::Core::Application::create_instance(*m_engine_core);
			m_engine_app->parse_command_line(argc, argv);

			// TODO: use command line to set up configs!
			Vadon::Core::CoreConfiguration engine_config;
			if (m_engine_core->initialize(engine_config) == false)
			{
				Vadon::Core::Logger::log_error("Failed to initialize engine core!\n");
				return false;
			}

			// Initialize the editor
			VadonApp::Core::Configuration app_config;
			if (m_engine_app->initialize(app_config) == false)
			{
				Vadon::Core::Logger::log_error("Failed to initialize engine application!\n");
				return false;
			}

			// FIXME: this needs to be done here to ensure that the parent types are already available
			// Should refactor to instead have "auto-registering" (via macros on the declarations and static vars)
			// that enqueues type registry metadata, and then use one explicit call to process all of it.
			VadonDemo::Core::Core::register_types();

			if (load_project() == false)
			{
				return false;
			}

			m_core = std::make_unique<Core>(m_engine_app->get_engine_core());
			if(m_core->initialize(m_project_info) == false)
			{
				return false;
			}

			if (m_platform_interface.initialize() == false)
			{
				return false;
			}

			if (m_render_system.initialize() == false)
			{
				return false;
			}

			// FIXME: at the moment this has to be done as a separate, bespoke step
			// Should replace with actual configurable viewports!
			if (m_render_system.init_viewport() == false)
			{
				return false;
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
				
				// Update model
				m_game_model.update();

				// Update view
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
			m_engine_core->shutdown();

			Vadon::Core::Logger::log_message("Vadon Demo app successfully shut down.\n");
		}

		bool load_project()
		{
			// First validate the path
			Vadon::Core::EngineCoreInterface& engine_core = *m_engine_core;
			
			// Default path assumes we are in "bin" and "data" is next to it
			std::filesystem::path fs_root_path = (std::filesystem::current_path().parent_path() / "data").generic_string();
			
			// Check command line arg, load startup project if requested
			constexpr const char c_data_dir_arg[] = "data_dir";
			if (m_engine_app->has_command_line_arg(c_data_dir_arg) == true)
			{
				fs_root_path = std::filesystem::path(m_engine_app->get_command_line_arg(c_data_dir_arg)).generic_string();
			}

			if (Vadon::Core::Project::is_valid_project_path(fs_root_path.generic_string()) == false)
			{
				constexpr const char c_invalid_path_error[] = "Game demo: invalid project path!\n";

				// Assume we gave a path to the folder with the project file
				if (std::filesystem::is_directory(fs_root_path) == true)
				{
					fs_root_path = (fs_root_path / Vadon::Core::Project::c_project_file_name).generic_string();
					if (Vadon::Core::Project::is_valid_project_path(fs_root_path.generic_string()) == false)
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

			// Initialize with default properties
			m_project_info.custom_properties = GlobalConfiguration::get_default_properties();

			const std::string project_file_path = fs_root_path.generic_string();
			Vadon::Core::RawFileDataBuffer project_file_data;

			Vadon::Core::FileSystem& file_system = m_engine_core->get_system<Vadon::Core::FileSystem>();
			if (file_system.load_file(project_file_path, project_file_data) == false)
			{
				Vadon::Core::Logger::log_error(std::format("Cannot load project file at \"{}\"!\n", project_file_path));
				return false;
			}

			const bool is_raw_data = m_engine_app->has_command_line_arg("raw");

			Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_data, is_raw_data ? Vadon::Utilities::Serializer::Type::JSON : Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::READ);
			if (Vadon::Core::Project::serialize_project_data(*serializer, m_project_info) == false)
			{
				Vadon::Core::Logger::log_error(std::format("Invalid project file at \"{}\"!\n", project_file_path));
				return false;
			}
			m_project_info.root_path = fs_root_path.parent_path().generic_string();

			if (m_resource_db.initialize(m_project_info.root_path, is_raw_data) == false)
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

	Vadon::Core::EngineCoreInterface& GameCore::get_engine_core()
	{
		return *m_internal->m_engine_core;
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

	Core& GameCore::get_core()
	{
		return *m_internal->m_core;
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

	const Vadon::Core::Project& GameCore::get_project_info() const
	{
		return m_internal->m_project_info;
	}

	void GameCore::request_shutdown()
	{
		m_internal->request_shutdown();
	}
}