#include <VadonEditor/Core/AssetServer.hpp>

#include <VadonEditor/Core/Project/AssetExporter.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Database.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

#include <Vadon/Foundation/Editor/Network/Message/AssetServer.hpp>

namespace VadonEditor::Core
{
	struct AssetServer::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Vadon::Utilities::CommandLineParser m_command_line_parser;

		ProjectManager m_project_manager;
		Model::ResourceDatabase m_resource_database;
		AssetExporter m_exporter;

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

		void update()
		{
			m_exporter.update();
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

		bool start_exporter(std::string_view output_path)
		{
			// Set the exporter output
			if (m_exporter.initialize_output(output_path) == false)
			{
				return false;
			}

			// Start by exporting all project contents
			if (m_exporter.export_all_project_data() == false)
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

	bool AssetServer::start_exporter(std::string_view output_path)
	{
		return m_internal->start_exporter(output_path);
	}

	void AssetServer::update()
	{
		m_internal->update();
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
			(void)message_data;
		}
			break;
		case ::Vadon::Foundation::EditorMessageCategory::ASSET_SERVER:
		{
			// TODO: anything?
		}
			break;
		default:
			// TODO!!!
			break;
		}
	}
}