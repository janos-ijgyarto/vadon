#include <VadonEditor/Simulator/Simulator.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Plugin/Plugin.hpp>
#include <VadonEditor/Core/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/Simulator/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/Simulator/PluginInterface.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Simulator.hpp>

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTimer>

namespace
{
	class NullMetadataRegistry : public Vadon::Foundation::TypeMetadataRegistry
	{
	public:
		void register_type(const ::Vadon::Foundation::TypeInfo&) override {}

		size_t get_registered_type_count() const override { return 0; }
		::Vadon::Foundation::UUID get_type_uuid(size_t) const override { return ::Vadon::Foundation::UUID{}; }

		::Vadon::Foundation::TypeInfo get_type_info(const ::Vadon::Foundation::UUID&) const override { return ::Vadon::Foundation::TypeInfo{}; }
		::Vadon::Foundation::UUID get_type_property_uuid(const ::Vadon::Foundation::UUID&, size_t) const override { return ::Vadon::Foundation::UUID{}; }

		void set_type_metadata(const ::Vadon::Foundation::UUID&, const char*, const char*) override {}
		const char* get_type_metadata(const ::Vadon::Foundation::UUID&, const char*) const override { return nullptr; }

		void register_property(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::Property&, const ::Vadon::Foundation::UUID*) override {}
		::Vadon::Foundation::Property get_property_info(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&) const override { return ::Vadon::Foundation::Property{}; }
		::Vadon::Foundation::UUID get_property_type_list_entry(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&, size_t) const override { return ::Vadon::Foundation::UUID{}; }

		void set_property_metadata(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&, const char*, const char*) override {}
		const char* get_property_metadata(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&, const char*) const override { return nullptr; }
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
		NullMetadataRegistry m_metadata_registry;
	};

	struct SimulatorSettings
	{
		std::string plugin_path;
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
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginEntrypoint) m_entrypoint_func;
		VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginExit) m_exit_func;

		::Vadon::Foundation::EditorSimulatorPluginInterface* m_plugin_interface;
		QTimer m_plugin_timer;

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
			// TODO: anything?
			return true;
		}

		bool load_plugin()
		{
			VadonEditor::Core::PluginManager& plugin_manager = m_application.get_plugin_manager();
			const VadonEditor::Core::ProjectManager& project_manager = m_application.get_project_manager();
			const VadonEditor::Core::ProjectInfo& project_info = project_manager.get_project_info();

			Core::PluginInfo plugin_info;
			plugin_info.path = project_info.plugin_path;

			m_simulator_plugin = plugin_manager.load_plugin(plugin_info);
			if (m_simulator_plugin == Core::PluginManager::c_invalid_plugin_handle)
			{
				qCritical() << "Simulator failed to load plugin!";
				unload_plugin();
				return false;
			}

			// FIXME: wrap this in a more concise macro?
			m_entrypoint_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginEntrypoint)>(plugin_manager.get_plugin_function(m_simulator_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorPluginEntrypoint)));
			if (m_entrypoint_func == nullptr)
			{
				qCritical() << "Failed to get entrypoint function address!";
				unload_plugin();
				return false;
			}

			m_exit_func = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginExit)>(plugin_manager.get_plugin_function(m_simulator_plugin, VADONEDITOR_API_FUNCTION_NAME(VadonEditorPluginExit)));
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

		bool run_plugin()
		{
			if (load_plugin() == false)
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
					case ::Vadon::Foundation::EditorMessageCategory::SIMULATOR:
					{
						const ::Vadon::Foundation::EditorSimulatorMessageHeader* simulator_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorSimulatorMessageHeader*>(message_reader.get_current_message_data());
						switch (simulator_message_header->message_type)
						{
						case ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_SHUTDOWN:
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
				::Vadon::Foundation::EditorSimulatorMessageInit init_message;
				init_message.message_type = ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_INIT;
				init_message.error_code = 0;

				VadonEditor::Network::MessageSerializer serializer;
				serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::SIMULATOR, init_message);

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

		bool run_simulator(const SimulatorSettings& settings)
		{
			const Core::Configuration& configuration = m_application.get_configuration();

			Core::ProjectManager& project_manager = m_application.get_project_manager();
			const Core::ProjectInfo& project_info = project_manager.get_project_info();

			if (project_info.plugin_path.isEmpty() == true)
			{
				qWarning() << "Project has no runnable simulator plugin set!";
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

				QString program_path = QCoreApplication::applicationFilePath();
				m_simulator_process.setProgram(program_path);

				QStringList arguments{ QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::IS_SIMULATOR)) };
				arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::STARTUP_PROJECT_PATH)));
				arguments.push_back(project_info.get_project_file_path());

				if (settings.debug_break_on_init == true)
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
				// We are the simulator, load plugin!
				if (run_plugin() == false)
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

					::Vadon::Foundation::EditorSimulatorMessageShutdown shutdown_message;
					shutdown_message.message_type = ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_SHUTDOWN;

					message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::SIMULATOR, shutdown_message);

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
	};

	Simulator::~Simulator() = default;

	bool Simulator::run_simulator(const SimulatorSettings& settings)
	{
		return m_internal->run_simulator(settings);
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