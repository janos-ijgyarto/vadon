#include <VadonEditor/Simulator/Simulator.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Plugin/Plugin.hpp>
#include <VadonEditor/Core/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Editor/Simulator/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/Simulator/PluginInterface.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Simulator.hpp>

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTimer>

namespace
{
	struct SimulatorToolchainConfig
	{
		QByteArray temp_path;
	};

	// NOTE: null implementation of plugin in case no plugin path was provided (useful for testing)
	class NullPlugin : public Vadon::Foundation::EditorSimulatorPluginInterface
	{
	public:
		NullPlugin(VadonEditor::Core::Application& application)
			: Vadon::Foundation::EditorSimulatorPluginInterface(application.get_simulator())
		{
		}

		bool initialize(const char* project_path) override
		{
			// TODO: test sending message to editor!
			Q_UNUSED(project_path);
			return true;
		}

		void update() override
		{
			// TODO: anything?
		}

		void shutdown() override
		{
			// TODO: anything?
		}

		void process_message_from_editor(const char* data, size_t size) override
		{
			::Vadon::Foundation::EditorMessageReader message_reader(data, size);
			switch (message_reader.get_current_category())
			{
			case ::Vadon::Foundation::EditorMessageCategory::TEST:
			{
				// Send back a test message of our own
				::Vadon::Foundation::EditorMessageTest test_message_in;

				qInfo() << "Server test message received: number = " << test_message_in.number << ", other number = " << test_message_in.other_number;

				::Vadon::Foundation::EditorMessageTest test_message_out;
				test_message_out.number = 2 * test_message_in.number;
				test_message_out.other_number = 3 * test_message_in.other_number;

				VadonEditor::Network::MessageSerializer serializer;
				serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::TEST, test_message_out);

				m_simulator.dispatch_message_to_editor(serializer.get_buffer().data(), serializer.get_buffer().size());
			}
			break;
			}
		}

		void editor_connected() override
		{
			// TODO
		}

		void editor_disconnected()  override
		{
			QCoreApplication::quit();
		}

		const Vadon::Foundation::TypeMetadataRegistry& get_metadata_registry() const { return m_metadata_registry; }
	private:
		::Vadon::Foundation::NullMetadataRegistry m_metadata_registry;
	};
}

namespace VadonEditor::Simulator
{
	struct Simulator::Internal
	{
		Core::Application& m_application;

		// TODO: split contents into editor and simulator objects (only one or the other will be initialized
		QProcess m_simulator_process;

		Core::PluginHandle m_simulator_plugin;
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorSimulatorPluginEntrypoint) m_entrypoint_func;
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorSimulatorPluginExit) m_exit_func;

		::Vadon::Foundation::EditorSimulatorPluginInterface* m_plugin_interface;

		// NOTE: have to create an object where we can store the UTF8-converted QStrings
		// because the struct used by the plugin uses const char*
		SimulatorToolchainConfig m_toolchain_config;

		QTimer m_plugin_timer;

		SimulatorSettingsData m_settings;

		Internal(Core::Application& application)
			: m_application(application)
			, m_simulator_plugin(Core::PluginManager::c_invalid_plugin_handle)
			, m_entrypoint_func(nullptr)
			, m_exit_func(nullptr)
			, m_plugin_interface(nullptr)
		{
		}

		bool initialize()
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			if (configuration.mode == Core::ApplicationMode::EDITOR)
			{
				QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::received_message,
					[this](const QByteArray& data)
					{
						::Vadon::Foundation::EditorMessageReader message_reader(data.constData(), data.size());
						switch (message_reader.get_current_category())
						{
						case ::Vadon::Foundation::EditorMessageCategory::SIMULATOR:
						{
							const ::Vadon::Foundation::EditorSimulatorMessageHeader* simulator_header = reinterpret_cast<const ::Vadon::Foundation::EditorSimulatorMessageHeader*>(message_reader.get_current_message_data());
							switch (simulator_header->message_type)
							{
							case ::Vadon::Foundation::EditorSimulatorMessageType::REGISTER_SETTING:
							{
								const ::Vadon::Foundation::EditorSimulatorMessageRegisterSetting* register_setting_message = reinterpret_cast<const ::Vadon::Foundation::EditorSimulatorMessageRegisterSetting*>(message_reader.get_current_message_data());
								register_setting(register_setting_message);
							}
							break;
							}
						}
						break;
						}
					}
				);
			}

			return true;
		}

		bool load_plugin(const QString& configuration_name)
		{
			VadonEditor::Core::PluginManager& plugin_manager = m_application.get_plugin_manager();
			const VadonEditor::Core::ProjectManager& project_manager = m_application.get_project_manager();
			const VadonEditor::Core::EditorProject& editor_project = project_manager.get_editor_project();

			const Core::EditorPluginInfo* editor_plugin_info = editor_project.find_plugin_entry(configuration_name);
			if (editor_plugin_info == nullptr)
			{
				qCritical() << "Invalid setting for project editor plugin!";
				return false;
			}

			Core::PluginInfo plugin_info;
			plugin_info.path = editor_plugin_info->path;

			m_simulator_plugin = plugin_manager.load_plugin(plugin_info);
			if (m_simulator_plugin == Core::PluginManager::c_invalid_plugin_handle)
			{
				qCritical() << "Simulator failed to load plugin!";
				unload_plugin();
				return false;
			}

			// FIXME: wrap this in a more concise macro?
			m_entrypoint_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorSimulatorPluginEntrypoint)>(plugin_manager.get_plugin_function(m_simulator_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorSimulatorPluginEntrypoint)));
			if (m_entrypoint_func == nullptr)
			{
				qCritical() << "Failed to get entrypoint function address!";
				unload_plugin();
				return false;
			}

			m_exit_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorSimulatorPluginExit)>(plugin_manager.get_plugin_function(m_simulator_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorSimulatorPluginExit)));
			if (m_exit_func == nullptr)
			{
				qCritical() << "Failed to get exit function address!";
				unload_plugin();
				return false;
			}

			if (m_entrypoint_func != nullptr)
			{
				// Plugin will create its interface and return it to us
				m_plugin_interface = m_entrypoint_func(&m_application.get_simulator());
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
			const VadonEditor::Core::SourceProjectInfo& source_project_info = project_manager.get_source_project().info;
			if (m_plugin_interface->initialize(source_project_info.get_project_file_path().toUtf8().constData()) == false)
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
						if (plugin_message_header->plugin_type != ::Vadon::Foundation::EditorPluginMessageSource::SIMULATOR)
						{
							return;
						}

						switch (plugin_message_header->message_type)
						{
						case ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_SHUTDOWN:
						{
							// TODO: run shutdown code in plugin
							// Stop timer so it doesn't try to update during shutdown
							m_plugin_timer.stop();
							m_application.request_quit(0);
							return;
						}
						break;
						}
					}
					break;
					}

					// Pass on to plugin
					// TODO: any messages that we should handle in the simulator?
					m_plugin_interface->process_message_from_editor(data.data(), data.size());
				}
			);

			// The editor is also connected by this point, so we can notify the plugin
			m_plugin_interface->editor_connected();

			// Send message back to editor
			{
				::Vadon::Foundation::EditorPluginMessageInit init_message;
				init_message.plugin_type = ::Vadon::Foundation::EditorPluginMessageSource::SIMULATOR;
				init_message.message_type = ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_INIT;
				init_message.error_code = 0;

				VadonEditor::Network::MessageSerializer serializer;
				serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLUGIN, init_message);

				m_application.get_simulator().dispatch_message_to_editor(serializer.get_buffer().data(), serializer.get_buffer().size());
			}

			return true;
		}

		void update()
		{
			m_plugin_interface->update();
		}

		void shutdown()
		{
			// Make sure we stop the simulator
			stop_simulator();
		}

		bool run_simulator(const SimulatorStartupOptions& startup_options)
		{
			const Core::Configuration& configuration = m_application.get_configuration();

			Core::ProjectManager& project_manager = m_application.get_project_manager();
			const Core::EditorProject& editor_project = project_manager.get_editor_project();

			const Core::EditorPluginInfo* editor_plugin_info = editor_project.find_plugin_entry(startup_options.configuration_name);
			if (editor_plugin_info == nullptr)
			{
				qCritical() << "Invalid setting for project editor plugin!";
				return false;
			}

			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_simulator_process.state() != QProcess::NotRunning)
				{
					qWarning() << "Simulator already running!";
					return true;
				}

				// Clear previous simulator settings
				m_settings.clear();

				QString program_path = QCoreApplication::applicationFilePath();
				m_simulator_process.setProgram(program_path);

				QStringList arguments{ QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::IS_SIMULATOR)) };

				arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::STARTUP_PROJECT_PATH)));
				arguments.push_back(editor_project.info.get_project_file_path());

				arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::PLUGIN_CONFIG_NAME)));
				arguments.push_back(startup_options.configuration_name);

				if (startup_options.debug_break_on_init == true)
				{
					arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::DEBUG_BREAK_ON_INIT)));
				}

				m_simulator_process.setArguments(arguments);

				QObject::connect(&m_simulator_process, &QProcess::aboutToClose, [this]() { cleanup_process(); });
				QObject::connect(&m_simulator_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error) { process_error(error); });
				
				// TODO: connect to standard outputs as well?

				m_simulator_process.start(QIODevice::ReadOnly);
			}
			break;
			case Core::ApplicationMode::SIMULATOR:
			{
				// Cache the export path so the asset server can request it
				QString output_path = m_application.get_project_manager().get_editor_project().info.output_path;
				if (output_path.isEmpty() == true)
				{
					qCritical() << "Simulator needs valid output path!";
					return false;
				}

				m_toolchain_config.temp_path = QDir::cleanPath(output_path + "/temp").toUtf8();

				// We are the simulator, load plugin!
				if (run_plugin(startup_options.configuration_name) == false)
				{
					return false;
				}

				// Start timer to update the plugin
				QObject::connect(&m_plugin_timer, &QTimer::timeout,
					[this]()
					{
						update();
					}
				);
				m_plugin_timer.start();
			}
			break;
			}

			qDebug() << "Simulator started";

			return true;
		}

		bool is_running() const
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_simulator_process.state() != QProcess::NotRunning)
				{
					return true;
				}
			}
			break;
			case Core::ApplicationMode::SIMULATOR:
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

		void stop_simulator()
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			switch (configuration.mode)
			{
			case Core::ApplicationMode::EDITOR:
			{
				if (m_simulator_process.state() == QProcess::ProcessState::NotRunning)
				{
					// Simulator is already turned off
					return;
				}

				// Message process to make sure it shuts down
				{
					VadonEditor::Network::MessageSerializer message_serializer;

					::Vadon::Foundation::EditorPluginMessageShutdown shutdown_message;
					shutdown_message.plugin_type = ::Vadon::Foundation::EditorPluginMessageSource::SIMULATOR;
					shutdown_message.message_type = ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_SHUTDOWN;

					message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLUGIN, shutdown_message);

					m_application.get_network_system().send_message(message_serializer);
				}

				// Wait for process to finish
				if (m_simulator_process.waitForFinished() == false)
				{
					qCritical() << "Error shutting down simulator!";
				}

				if (m_simulator_process.exitStatus() == QProcess::ExitStatus::NormalExit)
				{
					qDebug() << "Simulator process exited with " << m_simulator_process.exitCode();
				}
				else
				{
					qCritical() << "Simulator process crashed!";
				}
			}
			break;
			case Core::ApplicationMode::SIMULATOR:
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
			if (m_simulator_plugin != Core::PluginManager::c_invalid_plugin_handle)
			{
				m_application.get_plugin_manager().unload_plugin(m_simulator_plugin);
				m_simulator_plugin = Core::PluginManager::c_invalid_plugin_handle;
			}
		}

		void cleanup_process()
		{
			// TODO: anything else?
			qInfo() << "Simulator process shutting down";
		}

		void process_error(QProcess::ProcessError error)
		{
			// TODO: anything else?
			qCritical() << "Error running simulator process: " << error;
		}

		::Vadon::Foundation::SimulatorToolchainConfiguration get_toolchain_configuration() const
		{
			::Vadon::Foundation::SimulatorToolchainConfiguration toolchain_config;
			toolchain_config.temp_path = m_toolchain_config.temp_path.constData();

			return toolchain_config;
		}

		const SimulatorSettingsData& get_settings() const
		{
			return m_settings;
		}

		void register_setting(const ::Vadon::Foundation::EditorSimulatorMessageRegisterSetting* register_message)
		{
			SimulatorSetting new_setting;
			new_setting.id = Utilities::vadon_uuid_to_qt_uuid(register_message->setting.id);
			new_setting.type = Utilities::vadon_uuid_to_qt_uuid(register_message->setting.type);

			const char* raw_data_ptr = reinterpret_cast<const char*>(register_message) + sizeof(::Vadon::Foundation::EditorSimulatorMessageRegisterSetting);

			new_setting.label = QString::fromLocal8Bit(raw_data_ptr, register_message->label_length);

			const ::Vadon::Foundation::BaseType setting_data_type = Core::TypeData::get_base_type(new_setting.type);
			switch (setting_data_type)
			{
			// TODO: add support for other types!
			// TODO2: also have a way to add a "button" instead of a toggle
			case ::Vadon::Foundation::BaseType::BOOL:
			{
				new_setting.value = false;
			}
			break;
			default:
			{
				qWarning() << "Simulator setting not supported for type" << new_setting.type;
				return;
			}
			}

			for (const SimulatorSetting& current_setting : m_settings.settings)
			{
				if (current_setting.id == new_setting.id)
				{
					qWarning() << "Simulator setting already registered with ID" << new_setting.id;
					return;
				}
			}

			m_settings.settings.push_back(new_setting);
		}

		void set_setting_value(const QUuid& id, const QVariant& value)
		{
			const Core::Configuration& configuration = m_application.get_configuration();
			if(configuration.mode != Core::ApplicationMode::EDITOR)
			{
				return;
			}

			for (SimulatorSetting& current_setting : m_settings.settings)
			{
				if (current_setting.id == id)
				{
					current_setting.value = value;

					// Message simulator to make sure it shuts down
					VadonEditor::Network::MessageSerializer message_serializer;

					::Vadon::Foundation::EditorSimulatorMessageUpdateSetting update_message;
					update_message.setting_id = Utilities::qt_uuid_to_vadon_uuid(current_setting.id);
					update_message.message_type = ::Vadon::Foundation::EditorSimulatorMessageType::UPDATE_SETTING;

					const ::Vadon::Foundation::BaseType setting_data_type = Core::TypeData::get_base_type(current_setting.type);
					QByteArray message_data_buffer;
					switch (setting_data_type)
					{
					// TODO: add support for other types!
					// TODO2: also have a way to add a "button" instead of a toggle
					case ::Vadon::Foundation::BaseType::BOOL:
					{
						const bool bool_value = value.toBool();
						message_data_buffer.append(bool_value);
					}
						break;
					}

					update_message.data_size = message_data_buffer.size();

					char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::SIMULATOR, sizeof(::Vadon::Foundation::EditorSimulatorMessageUpdateSetting) + message_data_buffer.size());

					memcpy(message_data, &update_message, sizeof(::Vadon::Foundation::EditorSimulatorMessageUpdateSetting));
					memcpy(message_data + sizeof(::Vadon::Foundation::EditorSimulatorMessageUpdateSetting), message_data_buffer.constData(), message_data_buffer.size());

					m_application.get_network_system().send_message(message_serializer);

					return;
				}
			}

			qWarning() << "Simulator setting not found:" << id;
		}
	};

	Simulator::~Simulator() = default;

	bool Simulator::run_simulator(const SimulatorStartupOptions& startup_options)
	{
		return m_internal->run_simulator(startup_options);
	}

	bool Simulator::is_running() const
	{
		return m_internal->is_running();
	}

	void Simulator::stop_simulator()
	{
		m_internal->stop_simulator();
	}

	::Vadon::Foundation::EditorSimulatorPluginInterface* Simulator::get_plugin_interface() const
	{
		return m_internal->m_plugin_interface;
	}

	void Simulator::dispatch_message_to_editor(const char* data, size_t size)
	{
		m_internal->m_application.get_network_system().send_message(QByteArrayView(data, size));
	}

	::Vadon::Foundation::SimulatorToolchainConfiguration Simulator::get_toolchain_configuration() const
	{
		return m_internal->get_toolchain_configuration();
	}

	const SimulatorSettingsData& Simulator::get_settings() const
	{
		return m_internal->get_settings();
	}

	void Simulator::set_setting_value(const QUuid& id, const QVariant& value)
	{
		m_internal->set_setting_value(id, value);
	}

	Simulator::Simulator(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{
	}

	bool Simulator::initialize()
	{
		if (m_internal->initialize() == false)
		{
			return false;
		}

		return true;
	}

	void Simulator::shutdown()
	{
		m_internal->shutdown();
	}
}