#include <VadonEditor/Core/Asset/AssetServer.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Plugin/Plugin.hpp>
#include <VadonEditor/Core/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/Asset/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/Asset/PluginInterface.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>
#include <Vadon/Foundation/Editor/Network/Message/AssetServer.hpp>

#include <QCoreApplication>
#include <QProcess>

namespace VadonEditor::Core
{
	struct AssetServer::Internal 
	{
		Core::Application& m_application;

		// TODO: split contents into editor and asset server objects (only one or the other will be initialized)
		QProcess m_asset_server_process;

		Core::PluginHandle m_asset_server_plugin;
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorAssetServerPluginEntrypoint) m_entrypoint_func;
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorAssetServerPluginExit) m_exit_func;

		::Vadon::Foundation::EditorAssetServerPluginInterface* m_plugin_interface;

		Internal(Core::Application& application)
			: m_application(application)
			, m_asset_server_plugin(Core::PluginManager::c_invalid_plugin_handle)
			, m_entrypoint_func(nullptr)
			, m_exit_func(nullptr)
			, m_plugin_interface(nullptr)
		{
		}

		bool initialize()
		{
			// TODO: anything?
			return true;
		}

		bool load_plugin(const QString& configuration_name)
		{
			VadonEditor::Core::PluginManager& plugin_manager = m_application.get_plugin_manager();
			const VadonEditor::Core::ProjectManager& project_manager = m_application.get_project_manager();
			const VadonEditor::Core::ProjectInfo& project_info = project_manager.get_project_info();

			const Core::EditorPluginInfo* editor_plugin_info = project_info.find_plugin_entry(configuration_name);
			if (editor_plugin_info == nullptr)
			{
				qCritical() << "Invalid setting for project editor plugin!";
				return false;
			}

			Core::PluginInfo plugin_info;
			plugin_info.path = editor_plugin_info->path;

			m_asset_server_plugin = plugin_manager.load_plugin(plugin_info);
			if (m_asset_server_plugin == Core::PluginManager::c_invalid_plugin_handle)
			{
				qCritical() << "Asset server failed to load plugin!";
				unload_plugin();
				return false;
			}

			// FIXME: wrap this in a more concise macro?
			m_entrypoint_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorAssetServerPluginEntrypoint)>(plugin_manager.get_plugin_function(m_asset_server_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorAssetServerPluginEntrypoint)));
			if (m_entrypoint_func == nullptr)
			{
				qCritical() << "Failed to get entrypoint function address!";
				unload_plugin();
				return false;
			}

			m_exit_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorAssetServerPluginExit)>(plugin_manager.get_plugin_function(m_asset_server_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorAssetServerPluginExit)));
			if (m_exit_func == nullptr)
			{
				qCritical() << "Failed to get exit function address!";
				unload_plugin();
				return false;
			}

			if (m_entrypoint_func != nullptr)
			{
				// Plugin will create its interface and return it to us
				m_plugin_interface = m_entrypoint_func(&m_application.get_asset_server());
			}

			return true;
		}

		bool run_plugin(const QString& configuration_name)
		{
			if (load_plugin(configuration_name) == false)
			{
				return false;
			}

			if (m_plugin_interface == nullptr)
			{
				qCritical() << "No plugin interface object was created!";
				return false;
			}

			const VadonEditor::Core::ProjectManager& project_manager = m_application.get_project_manager();
			const VadonEditor::Core::ProjectInfo& project_info = project_manager.get_project_info();
			if (m_plugin_interface->initialize(project_info.get_project_file_path().toUtf8().constData()) == false)
			{
				qCritical() << "Plugin failed to initialize!";
				return false;
			}

			// Connect network signals
			QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::disconnected_from_server,
				[this]()
				{
					m_plugin_interface->editor_disconnected();
				}
			);

			QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::received_message,
				[this](const QByteArray& data)
				{
					::Vadon::Foundation::EditorMessageReader message_reader(data.constData(), data.size());

					switch (message_reader.get_current_category())
					{
					case ::Vadon::Foundation::EditorMessageCategory::PLUGIN:
					{
						const ::Vadon::Foundation::EditorPluginMessageHeader* plugin_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorPluginMessageHeader*>(message_reader.get_current_message_data());
						if (plugin_message_header->plugin_type != ::Vadon::Foundation::EditorPluginMessageSource::ASSET_SERVER)
						{
							return;
						}

						switch (plugin_message_header->message_type)
						{
						case ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_SHUTDOWN:
						{
							// TODO: run shutdown code in plugin
							// Stop timer so it doesn't try to update during shutdown
							m_application.request_quit(0);
							return;
						}
						break;
						}
					}
					break;
					}

					// Pass on to plugin
					// TODO: any messages that we should handle in the asset server?
					m_plugin_interface->process_message_from_editor(data.data(), data.size());
				}
			);

			// The editor is also connected by this point, so we can notify the plugin
			m_plugin_interface->editor_connected();

			// Send message back to editor
			{
				::Vadon::Foundation::EditorPluginMessageInit init_message;
				init_message.plugin_type = ::Vadon::Foundation::EditorPluginMessageSource::ASSET_SERVER;
				init_message.message_type = ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_INIT;
				init_message.error_code = 0;

				VadonEditor::Network::MessageSerializer serializer;
				serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLUGIN, init_message);

				m_application.get_asset_server().dispatch_message_to_editor(serializer.get_buffer().data(), serializer.get_buffer().size());
			}

			return true;
		}

		void shutdown()
		{
			// Make sure we stop the asset server
			stop_asset_server();
		}

		bool run_asset_server(const AssetServerSettings& settings)
		{
			const Core::Configuration& configuration = m_application.get_configuration();

			Core::ProjectManager& project_manager = m_application.get_project_manager();
			const Core::ProjectInfo& project_info = project_manager.get_project_info();

			const Core::EditorPluginInfo* editor_plugin_info = project_info.find_plugin_entry(settings.configuration_name);
			if (editor_plugin_info == nullptr)
			{
				qCritical() << "Invalid setting for project editor plugin!";
				return false;
			}

			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_asset_server_process.state() != QProcess::NotRunning)
				{
					qWarning() << "Asset server already running!";
					return true;
				}

				QString program_path = QCoreApplication::applicationFilePath();
				m_asset_server_process.setProgram(program_path);

				QStringList arguments{ QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::IS_ASSET_SERVER)) };
				arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::STARTUP_PROJECT_PATH)));
				arguments.push_back(project_info.get_project_file_path());

				arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::PLUGIN_CONFIG_NAME)));
				arguments.push_back(settings.configuration_name);

				if (settings.debug_break_on_init == true)
				{
					arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::DEBUG_BREAK_ON_INIT)));
				}

				m_asset_server_process.setArguments(arguments);

				QObject::connect(&m_asset_server_process, &QProcess::aboutToClose, [this]() { cleanup_process(); });
				QObject::connect(&m_asset_server_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error) { process_error(error); });

				// TODO: connect to standard outputs as well?

				m_asset_server_process.start(QIODevice::ReadOnly);
			}
			break;
			case Core::ApplicationMode::ASSET_SERVER:
			{
				// We are the asset server, load plugin!
				if (run_plugin(settings.configuration_name) == false)
				{
					return false;
				}
			}
			break;
			}

			qDebug() << "Asset server started";

			return true;
		}

		bool is_running() const
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_asset_server_process.state() != QProcess::NotRunning)
				{
					return true;
				}
			}
			break;
			case Core::ApplicationMode::ASSET_SERVER:
			{
				if (m_plugin_interface != nullptr)
				{
					return true;
				}
			}
			break;
			}

			return false;
		}

		void stop_asset_server()
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_asset_server_process.state() == QProcess::ProcessState::NotRunning)
				{
					// Asset server is already turned off
					return;
				}

				// Message process to make sure it shuts down
				{
					VadonEditor::Network::MessageSerializer message_serializer;

					::Vadon::Foundation::EditorPluginMessageShutdown shutdown_message;
					shutdown_message.plugin_type = ::Vadon::Foundation::EditorPluginMessageSource::ASSET_SERVER;
					shutdown_message.message_type = ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_SHUTDOWN;

					message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLUGIN, shutdown_message);

					m_application.get_network_system().send_message(message_serializer);
				}

				// Wait for process to finish
				if (m_asset_server_process.waitForFinished() == false)
				{
					qCritical() << "Error shutting down asset server!";
				}

				if (m_asset_server_process.exitStatus() == QProcess::ExitStatus::NormalExit)
				{
					qDebug() << "Asset server process exited with " << m_asset_server_process.exitCode();
				}
				else
				{
					qCritical() << "Asset server process crashed!";
				}
			}
			break;
			case Core::ApplicationMode::ASSET_SERVER:
			{
				if (m_plugin_interface == nullptr)
				{
					return;
				}

				// Call shutdown on the plugin itself
				m_plugin_interface->shutdown();

				if (m_exit_func != nullptr)
				{
					// Pass the interface back to plugin (it knows how it was allocated)
					m_exit_func(m_plugin_interface);
				}
				else
				{
					delete m_plugin_interface;
				}
				m_plugin_interface = nullptr;

				unload_plugin();

				m_entrypoint_func = nullptr;
				m_exit_func = nullptr;
			}
			break;
			}
		}

		void unload_plugin()
		{
			if (m_asset_server_plugin != Core::PluginManager::c_invalid_plugin_handle)
			{
				m_application.get_plugin_manager().unload_plugin(m_asset_server_plugin);
				m_asset_server_plugin = Core::PluginManager::c_invalid_plugin_handle;
			}
		}

		void cleanup_process()
		{
			// TODO: anything else?
			qInfo() << "Asset server process shutting down";
		}

		void process_error(QProcess::ProcessError error)
		{
			// TODO: anything else?
			qCritical() << "Error running asset server process: " << error;
		}

		void export_project_data(const QString& output_path)
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			if (configuration.mode != Core::ApplicationMode::EDITOR)
			{
				return;
			}

			// Send message to plugin
			VadonEditor::Network::MessageSerializer serializer;

			::Vadon::Foundation::EditorAssetServerMessageExportData export_data_message;
			export_data_message.message_type = ::Vadon::Foundation::EditorAssetServerMessageType::EXPORT_DATA;
			export_data_message.output_path_length = output_path.size();

			// FIXME: create a more elegant way to add a string to a message
			char* message_data = serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::ASSET_SERVER, sizeof(::Vadon::Foundation::EditorAssetServerMessageExportData) + export_data_message.output_path_length);
			memcpy(message_data, &export_data_message, sizeof(::Vadon::Foundation::EditorAssetServerMessageExportData));
			memcpy(message_data + sizeof(::Vadon::Foundation::EditorAssetServerMessageExportData), output_path.toUtf8().constData(), export_data_message.output_path_length);

			m_application.get_network_system().send_message(serializer);
		}
	};

	AssetServer::~AssetServer() = default;

	void AssetServer::dispatch_message_to_editor(const char* data, size_t size)
	{
		m_internal->m_application.get_network_system().send_message(QByteArrayView(data, size));
	}

	bool AssetServer::run_asset_server(const AssetServerSettings& settings)
	{
		return m_internal->run_asset_server(settings);
	}

	bool AssetServer::is_running() const
	{
		return m_internal->is_running();
	}

	void AssetServer::stop_asset_server()
	{
		m_internal->stop_asset_server();
	}

	void AssetServer::export_project_data(const QString& output_path)
	{
		m_internal->export_project_data(output_path);
	}

	AssetServer::AssetServer(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{

	}

	bool AssetServer::initialize()
	{
		if (m_internal->initialize() == false)
		{
			return false;
		}

		return true;
	}

	void AssetServer::shutdown()
	{
		m_internal->shutdown();
	}
}