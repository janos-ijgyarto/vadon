#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Settings.hpp>

#include <VadonEditor/Network/MessageSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/PluginInterface.hpp>
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

		void update() override
		{
			// TODO: anything?
		}

		void process_message_from_editor(const ::Vadon::Foundation::EditorMessageHeader& header, const void* data) override
		{
			const ::Vadon::Foundation::EditorMessageCategory category = static_cast<::Vadon::Foundation::EditorMessageCategory>(header.category);
			switch (category)
			{
			case ::Vadon::Foundation::EditorMessageCategory::TEST:
			{
				// Send back a test message of our own
				::Vadon::Foundation::EditorTestMessage test_message_in;
				VadonEditor::Network::MessageSerializer::parse_message(header, data, test_message_in);

				qInfo() << "Server test message received: number = " << test_message_in.number << ", other number = " << test_message_in.other_number << Qt::endl;

				::Vadon::Foundation::EditorTestMessage test_message_out;
				test_message_out.number = 2 * test_message_in.number;
				test_message_out.other_number = 3 * test_message_in.other_number;

				std::vector<char> message_buffer;
				VadonEditor::Network::MessageSerializer::write_message(test_message_out, message_buffer);

				m_simulator.dispatch_message_to_editor(message_buffer.data(), message_buffer.size());
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

		HMODULE m_library;
		VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(VadonEditorPluginEntrypoint) m_entrypoint_func;
		VADONEDITOR_SIMULATOR_API_FUNCTION_POINTER(VadonEditorPluginExit) m_exit_func;

		::Vadon::Foundation::EditorPluginInterface* m_plugin;

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
			const QString& plugin_path = m_application.get_settings().plugin_path;
			if (plugin_path.isEmpty() == false)
			{
				return launch_plugin(plugin_path);
			}
			else
			{
				qCritical() << "Did not provide a path to a plugin!" << Qt::endl;
				m_plugin = new NullPlugin(m_application);
				return true;
			}
		}

		bool launch_plugin(const QString& plugin_path)
		{
			m_library = LoadLibraryA(qPrintable(plugin_path));
			if (m_library == NULL)
			{
				qCritical() << "Failed to load library at \"" << plugin_path << "\"" << Qt::endl;
				return false;
			}

			m_entrypoint_func = VADONEDITOR_SIMULATOR_GET_API_FUNCTION_POINTER(m_library, VadonEditorPluginEntrypoint);
			if (m_entrypoint_func == nullptr)
			{
				qCritical() << "Failed to get entrypoint function address!" << Qt::endl;
				return false;
			}

			m_exit_func = VADONEDITOR_SIMULATOR_GET_API_FUNCTION_POINTER(m_library, VadonEditorPluginExit);
			if (m_exit_func == nullptr)
			{
				qCritical() << "Failed to get exit function address!" << Qt::endl;
				return false;
			}

			if (m_entrypoint_func != nullptr)
			{
				// Plugin will create its interface and return it to us
				m_plugin = m_entrypoint_func(&m_application.get_plugin_manager());
			}

			return true;
		}

		void update()
		{
			m_plugin->update();
		}

		void shutdown()
		{
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
	};

	PluginManager::PluginManager(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{
	}

	PluginManager::~PluginManager()
	{
		shutdown();
	}

	bool PluginManager::initialize()
	{
		return m_internal->initialize();
	}

	void PluginManager::update()
	{
		m_internal->update();
	}

	void PluginManager::shutdown()
	{
		m_internal->shutdown();
	}

	::Vadon::Foundation::EditorPluginInterface* PluginManager::get_plugin() const
	{
		return m_internal->m_plugin;
	}

	void PluginManager::dispatch_message_to_editor(const void* data, size_t size)
	{
		QByteArray message_buffer;
		message_buffer.append(reinterpret_cast<const char*>(data), size);

		m_internal->m_application.get_message_system().send_message(message_buffer);
	}
}