#include <VadonEditor/Core/AssetServer.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Database.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>
#include <Vadon/Model/Resource/File.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

#include <Vadon/Foundation/Editor/Network/Message/AssetServer.hpp>

#include <filesystem>

namespace
{
	struct AssetServerFileDatabaseEntry
	{
		std::string path;
	};

	class AssetServerExporter
	{
	public:
		AssetServerExporter(VadonEditor::Core::AssetServer& asset_server)
			: m_asset_server(asset_server)
		{

		}

		bool initialize()
		{
			// TODO: anything?
			return true;
		}

		bool export_data(std::string_view output_path)
		{
			// FIXME: currently we just export every single asset in the project
			// We should implement a system for including/excluding resources that end up in the final export
			Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

			// Create the export databases
			// FIXME: generalize this by iterating over all the asset types!
			Vadon::Core::FileDatabaseHandle resource_db_handle;
			Vadon::Core::FileDatabaseHandle asset_db_handle;

			{
				Vadon::Core::FileDatabaseInfo resource_db_info;
				resource_db_info.root_path = (std::filesystem::path(output_path) / "resources").generic_string();
				resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				resource_db_handle = file_system.create_database(resource_db_info);
			}

			{
				Vadon::Core::FileDatabaseInfo asset_db_info;
				asset_db_info.root_path = (std::filesystem::path(output_path) / "assets").generic_string();
				asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

				asset_db_handle = file_system.create_database(asset_db_info);
			}

			const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
			const std::vector<Vadon::Model::ResourceID> resource_list = resource_database.get_resource_list();

			Vadon::Model::ResourceSystem& engine_resource_system = engine_core.get_system<Vadon::Model::ResourceSystem>();

			Vadon::Core::RawFileDataBuffer source_file_data_buffer;
			Vadon::Core::RawFileDataBuffer exported_file_data_buffer;

			for (const Vadon::Model::ResourceID& current_resource_id : resource_list)
			{
				source_file_data_buffer.clear();				
				if (resource_database.load_resource_data(current_resource_id, source_file_data_buffer) == false)
				{
					Vadon::Core::Logger::log_error("Asset server exporter: failed to load resource file contents!\n");
					return false;
				}
				
				if (VadonEditor::Model::ResourceDatabase::sanitize_editor_resource_file(source_file_data_buffer) == false)
				{
					Vadon::Core::Logger::log_error("Asset server exporter: failed to sanitize resource file contents!\n");
					return false;
				}

				Vadon::Utilities::VariantDictionary resource_raw_data;
				{
					Vadon::Utilities::Serializer::Instance json_serializer = Vadon::Utilities::Serializer::create_serializer(source_file_data_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

					if (json_serializer->initialize() == false)
					{
						Vadon::Core::Logger::log_error("Asset server exporter: failed to initialize serializer while loading resource!\n");
						return false;
					}

					if (engine_resource_system.load_resource_raw_data(*json_serializer, resource_raw_data) == false)
					{
						// FIXME: allow graceful exit so the asset server doesn't crash?
						Vadon::Core::Logger::log_error("Asset server exporter: failed to load resource data!\n");
						return false;
					}

					if (json_serializer->finalize() == false)
					{
						Vadon::Core::Logger::log_error("Asset server exporter: failed to finalize import serializer!\n");
						return false;
					}
				}

				exported_file_data_buffer.clear();
				{
					Vadon::Utilities::Serializer::Instance binary_serializer = Vadon::Utilities::Serializer::create_serializer(exported_file_data_buffer, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::WRITE);
					if (binary_serializer->initialize() == false)
					{
						Vadon::Core::Logger::log_error("Asset server exporter: failed to initialize export serializer!\n");
						continue;
					}
					if (engine_resource_system.save_resource_raw_data(*binary_serializer, resource_raw_data) == false)
					{
						Vadon::Core::Logger::log_error("Asset server exporter: failed to serialize resource!\n");
						continue;
					}
					if (binary_serializer->finalize() == false)
					{
						Vadon::Core::Logger::log_error("Asset server exporter: failed to finalize export serializer!\n");
						continue;
					}
				}

				Vadon::Core::FileInfo file_info;
				file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
				file_info.size = static_cast<int>(exported_file_data_buffer.size());

				// Use hex representation of resource UUID as the file name
				file_info.path += Vadon::Utilities::uuid_to_hex_string(current_resource_id) + ".vdbin";

				// Add to asset library via the file DB
				file_system.add_existing_file(resource_db_handle, current_resource_id, file_info);
				if (file_system.save_file(resource_db_handle, current_resource_id, exported_file_data_buffer) == false)
				{
					Vadon::Core::Logger::log_error("Asset server exporter: failed to save resource to file!\n");
					continue;
				}

				const VadonEditor::Model::ResourceDatabaseEntry* resource_db_entry = resource_database.find_resource_entry(current_resource_id);

				if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Model::FileResource>(), resource_db_entry->base_info.type_id) == true)
				{
					// Resource points to a file, so we need to export that as well
					exported_file_data_buffer.clear();

					// Add to the export database
					Vadon::Core::FileInfo asset_file_info;
					asset_file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
					asset_file_info.size = 0; // FIXME: get the file size!

					// Same path as resource, but different DB root
					asset_file_info.path = file_info.path;

					file_system.add_existing_file(asset_db_handle, current_resource_id, file_info);

					// Copy from asset library to export destination
					const Vadon::Core::FileDatabaseHandle src_asset_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::ASSET_FILE);
					if (file_system.copy_file(src_asset_db_handle, current_resource_id, asset_db_handle, current_resource_id) == false)
					{
						Vadon::Core::Logger::log_error("Asset Server exporter: failed to copy asset file!\n");
						continue;
					}
				}
			}

			// Done exporting all data, remove the databases
			file_system.remove_database(resource_db_handle);
			file_system.remove_database(asset_db_handle);

			Vadon::Core::Logger::log_message(std::format("Asset Server exporter: exported project data to {}!\n", output_path));

			return true;
		}
	private:
		VadonEditor::Core::AssetServer& m_asset_server;
		std::unordered_map<::Vadon::Foundation::UUID, AssetServerFileDatabaseEntry> m_entry_lookup;
	};
}

namespace VadonEditor::Core
{
	struct AssetServer::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Vadon::Utilities::CommandLineParser m_command_line_parser;

		ProjectManager m_project_manager;
		Model::ResourceDatabase m_resource_database;
		AssetServerExporter m_exporter;

		Internal(AssetServer& asset_server, Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
			, m_resource_database(engine_core, m_project_manager)
			, m_exporter(asset_server)
		{

		}

		bool initialize()
		{
			if (m_exporter.initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool project_loaded()
		{
			// Import all resources
			if (m_resource_database.initialize() == false)
			{
				return false;
			}

			if (m_resource_database.import_project_resources() == false)
			{
				return false;
			}

			return true;
		}

		bool export_data(std::string_view output_path)
		{
			if (m_exporter.export_data(output_path) == false)
			{
				return false;
			}

			return true;
		}
	};

	AssetServer::AssetServer(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(*this, engine_core))
	{
	}

	AssetServer::~AssetServer() = default;

	void AssetServer::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	bool AssetServer::initialize()
	{
		return m_internal->initialize();
	}

	bool AssetServer::load_project(std::string_view root_path)
	{
		if (get_project_manager().load_project(get_engine_core(), root_path) == false)
		{
			return false;
		}

		return m_internal->project_loaded();
	}

	Vadon::Core::EngineCoreInterface& AssetServer::get_engine_core() { return m_internal->m_engine_core; }

	Vadon::Utilities::CommandLineParser& AssetServer::get_command_line_parser() { return m_internal->m_command_line_parser; }

	ProjectManager& AssetServer::get_project_manager() { return m_internal->m_project_manager; }

	Model::ResourceDatabase& AssetServer::get_resource_database() { return m_internal->m_resource_database; }

	void AssetServer::process_message(const char* data, size_t size)
	{
		::Vadon::Foundation::EditorMessageReader message_reader(data, size);
		const char* message_data = message_reader.get_current_message_data();
		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::PLUGIN:
		{
			// TODO: anything?
		}
			break;
		case ::Vadon::Foundation::EditorMessageCategory::ASSET_SERVER:
		{
			const ::Vadon::Foundation::EditorAssetServerMessageHeader* message_header = reinterpret_cast<const ::Vadon::Foundation::EditorAssetServerMessageHeader*>(message_data);
			switch (message_header->message_type)
			{
			case ::Vadon::Foundation::EditorAssetServerMessageType::EXPORT_DATA:
			{
				const ::Vadon::Foundation::EditorAssetServerMessageExportData* export_data_message = reinterpret_cast<const ::Vadon::Foundation::EditorAssetServerMessageExportData*>(message_data);
				const char* data_start = message_data + sizeof(::Vadon::Foundation::EditorAssetServerMessageExportData);

				std::string_view output_path_string(data_start, export_data_message->output_path_length);
				m_internal->export_data(output_path_string);
			}
			break;
			}
		}
			break;
		default:
			// TODO!!!
			break;
		}
	}
}