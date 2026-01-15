#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/PluginInterface.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Simulator.hpp>

#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>

// FIXME: ifdef this to only be used on Windows!
#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES // message types WM_*
//#define NOWINSTYLES   // window styles WS_*
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
//#define OEMRESOURCE // includes various image/GUI related stuff if defined
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
//#define NOUSER // all the window-related and lots of other stuff
//#define NONLS  // WideCharToMultiByte/MultiByteToWideChar, CP_UTF8, GetACP()
//#define NOMB   // MessageBox
#define NOMEMMGR
#define NOMETAFILE
//#define NOMSG // PeekMessage, etc.
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#define VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(_name) decltype(&(_name))
#define VADONEDITOR_SIMULATOR_GET_API_FUNCTION_POINTER(_library, _name) (VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(_name))GetProcAddress((_library), #_name);

#include <Windows.h>

#include <QDebug>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>

namespace
{
	class NullMetadataRegistry : public Vadon::Foundation::TypeMetadataRegistry
	{
	public:
		size_t get_registered_type_count() const override { return 0; }
		::Vadon::Foundation::UUID get_type_uuid(size_t) const override { return ::Vadon::Foundation::UUID{}; }

		size_t get_type_property_count(const ::Vadon::Foundation::UUID&) const override { return 0; }
		::Vadon::Foundation::UUID get_type_property_uuid(const ::Vadon::Foundation::UUID&, size_t) const override { return ::Vadon::Foundation::UUID{}; }

		void set_type_metadata(const ::Vadon::Foundation::UUID&, const char*, const char*) override {}
		const char* get_type_metadata(const ::Vadon::Foundation::UUID&, const char*) const override { return nullptr; }

		void set_property_metadata(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&, const char*, const char*) override {}
		const char* get_property_metadata(const ::Vadon::Foundation::UUID&, const ::Vadon::Foundation::UUID&, const char*) const override { return nullptr; }
	};

	// NOTE: null implementation of plugin in case no plugin path was provided (useful for testing)
	class NullPlugin : public Vadon::Foundation::EditorPluginInterface
	{
	public:
		NullPlugin(VadonEditor::Core::Application& application)
			: Vadon::Foundation::EditorPluginInterface(application.get_plugin_manager())
		{
		}

		bool initialize() override
		{
			// TODO: test sending message to editor!
			return true;
		}

		void update() override
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
	struct PluginManager::Internal
	{
		Core::Application& m_application;

		QProcess m_simulator_process;

		HMODULE m_library;
		VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(VadonEditorPluginEntrypoint) m_entrypoint_func;
		VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(VadonEditorPluginExit) m_exit_func;

		::Vadon::Foundation::EditorPluginInterface* m_plugin;
		QTimer m_plugin_timer;

		Internal(Core::Application& application)
			: m_application(application)
			, m_library(NULL)
			, m_entrypoint_func(nullptr)
			, m_exit_func(nullptr)
			, m_plugin(nullptr)
		{
		}

		bool initialize()
		{
			// TODO: anything?
			return true;
		}

		bool load_plugin()
		{
			const VadonEditor::Core::ProjectManager& project_manager = m_application.get_project_manager();
			const VadonEditor::Core::ProjectInfo& project_info = project_manager.get_project_info();

			const QFileInfo plugin_file_info(project_info.plugin_path);
			if (plugin_file_info.exists() == false
				|| plugin_file_info.isFile() == false
				|| plugin_file_info.suffix() != "dll")
			{
				return false;
			}

			DLL_DIRECTORY_COOKIE plugin_dir_cookie = AddDllDirectory(plugin_file_info.absolutePath().toStdWString().c_str());

			m_library = LoadLibraryExA(qPrintable(project_info.plugin_path), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
			if (m_library == NULL)
			{
				auto last_error = GetLastError();
				qCritical() << "Failed to load library at \"" << project_info.plugin_path << "\"" << "(last error: " << last_error << ")";
				RemoveDllDirectory(plugin_dir_cookie);
				return false;
			}

			RemoveDllDirectory(plugin_dir_cookie);

			m_entrypoint_func = VADONEDITOR_SIMULATOR_GET_API_FUNCTION_POINTER(m_library, VadonEditorPluginEntrypoint);
			if (m_entrypoint_func == nullptr)
			{
				qCritical() << "Failed to get entrypoint function address!";
				return false;
			}

			m_exit_func = VADONEDITOR_SIMULATOR_GET_API_FUNCTION_POINTER(m_library, VadonEditorPluginExit);
			if (m_exit_func == nullptr)
			{
				qCritical() << "Failed to get exit function address!";
				return false;
			}

			if (m_entrypoint_func != nullptr)
			{
				// Plugin will create its interface and return it to us
				m_plugin = m_entrypoint_func(&m_application.get_plugin_manager());
			}

			return true;
		}

		bool run_plugin()
		{
			if (load_plugin() == false)
			{
				return false;
			}

			if (m_plugin == nullptr)
			{
				qCritical() << "No plugin object was created!";
				return false;
			}

			if (m_plugin->initialize() == false)
			{
				qCritical() << "Plugin failed to initialize!";
				return false;
			}

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
							QCoreApplication::quit();
							return;
						}
						break;
						}
					}
					break;
					}

					// Pass on to plugin
					// TODO: any messages that we should handle in the simulator?
					m_plugin->process_message_from_editor(data.data(), data.size());
				}
			);

			return true;
		}

		void update()
		{
			m_plugin->update();
		}

		void shutdown()
		{
			const Core::CommandLineParameters& command_line_params = m_application.get_command_line_parameters();
			if (command_line_params.is_simulator == false)
			{
				if (m_simulator_process.state() != QProcess::ProcessState::NotRunning)
				{
					// Message process to make sure it shuts down
					{
						VadonEditor::Network::MessageSerializer message_serializer;

						::Vadon::Foundation::EditorSimulatorMessageShutdown shutdown_message;
						shutdown_message.message_type = ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_SHUTDOWN;

						message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::SIMULATOR, shutdown_message);

						QByteArray message_buffer;
						message_buffer.append(message_serializer.get_buffer().data(), message_serializer.get_buffer().size());

						m_application.get_network_system().send_message(message_buffer);
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
			}
			else
			{
				if (m_plugin == nullptr)
				{
					return;
				}

				if (m_exit_func != nullptr)
				{
					// Pass the interface back to plugin (it knows how it was allocated)
					m_exit_func(m_plugin);
				}
				else
				{
					delete m_plugin;
				}
				m_plugin = nullptr;

				if (m_library != NULL)
				{
					FreeLibrary(m_library);
					m_library = NULL;
				}
			}
		}

		bool run_simulator()
		{
			const Core::CommandLineParameters& command_line_params = m_application.get_command_line_parameters();

			Core::ProjectManager& project_manager = m_application.get_project_manager();
			const Core::ProjectInfo& project_info = project_manager.get_project_info();

			if (project_info.plugin_path.isEmpty() == true)
			{
				return false;
			}

			if (command_line_params.is_simulator == false)
			{
				if (m_simulator_process.state() != QProcess::NotRunning)
				{
					qWarning() << "Simulator already running!";
					return false;
				}

				QString program_path = QCoreApplication::applicationFilePath();
				m_simulator_process.setProgram(program_path);

				QStringList arguments{ "--simulator" };
				arguments.push_back("--startup-project");
				arguments.push_back(project_info.get_project_file_path());
				arguments.push_back("--debug-break-on-init");

				m_simulator_process.setArguments(arguments);

				QObject::connect(&m_simulator_process, &QProcess::aboutToClose, [this]() { cleanup_process(); });
				QObject::connect(&m_simulator_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error) { process_error(error); });

				QObject::connect(&m_simulator_process, &QProcess::readyReadStandardOutput,
					[this]()
					{
						qInfo() << qPrintable(m_simulator_process.readAllStandardOutput());
					}
				);

				m_simulator_process.start(QIODevice::ReadOnly);
			}
			else
			{
				// We are the simulator, load plugin!
				if (run_plugin() == false)
				{
					return false;
				}

				// Connect network signals
				QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::connected_to_server,
					[this]()
					{
						m_plugin->editor_connected();
					}
				);

				QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::disconnected_from_server,
					[this]()
					{
						m_plugin->editor_disconnected();
					}
				);

				// Start timer to update the plugin
				QObject::connect(&m_plugin_timer, &QTimer::timeout,
					[this]()
					{
						update();
					}
				);
				m_plugin_timer.start();
			}

			qDebug() << "Simulator started";

			return true;
		}

		void cleanup_process()
		{
			qInfo() << "Simulator process shutting down";
		}

		void process_error(QProcess::ProcessError error)
		{
			qCritical() << "Error running simulator process: " << error;
		}
	};

	PluginManager::PluginManager(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{
	}

	PluginManager::~PluginManager() = default;

	bool PluginManager::initialize()
	{
		if (m_internal->initialize() == false)
		{
			return false;
		}

		return true;
	}

	void PluginManager::shutdown()
	{
		m_internal->shutdown();
	}

	bool PluginManager::run_simulator()
	{
		return m_internal->run_simulator();
	}

	::Vadon::Foundation::EditorPluginInterface* PluginManager::get_plugin() const
	{
		return m_internal->m_plugin;
	}

	void PluginManager::dispatch_message_to_editor(const char* data, size_t size)
	{
		QByteArray message_buffer;
		message_buffer.append(data, size);

		m_internal->m_application.get_network_system().send_message(message_buffer);
	}
}