#include <VadonEditor/Core/Plugin/PluginManager.hpp>

#include <VadonEditor/Core/Plugin/Plugin.hpp>

#include <QFileInfo>

// FIXME: ideally we should use QLibrary,
// but we need to ensure we can load DLL dependencies from the same folder as the plugin itself
#if defined(Q_OS_WINDOWS)
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

#include <Windows.h>
#endif

namespace
{
#if defined(Q_OS_WINDOWS)
	using LibraryModule = HMODULE;
#else
	using LibraryModule = void*;
#endif

	struct PluginData
	{
		VadonEditor::Core::PluginInfo info;
		LibraryModule library_module;
		// TODO: anything else?
	};

	void* add_library_directory(const QString& path)
	{
#if defined(Q_OS_WINDOWS)
		DLL_DIRECTORY_COOKIE plugin_dir_cookie = AddDllDirectory(path.toStdWString().c_str());
		return plugin_dir_cookie;
#else
		Q_UNUSED(path);
		return nullptr;
#endif
	}

	void remove_library_directory(void* cookie)
	{
#if defined(Q_OS_WINDOWS)
		RemoveDllDirectory(cookie);
#else
		Q_UNUSED(cookie);
		// TODO
#endif
	}

	LibraryModule load_plugin_library(const QString& plugin_path)
	{
#if defined(Q_OS_WINDOWS)
		const QFileInfo plugin_file_info(plugin_path);
		if (plugin_file_info.exists() == false
			|| plugin_file_info.isFile() == false
			|| plugin_file_info.suffix() != "dll")
		{
			return NULL;
		}

		DLL_DIRECTORY_COOKIE plugin_dir_cookie = AddDllDirectory(plugin_file_info.absolutePath().toStdWString().c_str());

		LibraryModule library_module = LoadLibraryExA(qPrintable(plugin_path), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if (library_module == NULL)
		{
			auto last_error = GetLastError();
			qCritical() << "Failed to load library at \"" << plugin_path << "\"" << "(last error: " << last_error << ")";
			RemoveDllDirectory(plugin_dir_cookie);
			return NULL;
		}

		RemoveDllDirectory(plugin_dir_cookie);
		return library_module;
#else
		Q_UNUSED(plugin_path);
		return false;
#endif
	}

	void unload_plugin_library(LibraryModule library_module)
	{
#if defined(Q_OS_WINDOWS)
		// NOTE: for some reason here zero means "error"
		if (FreeLibrary(library_module) == 0)
		{
			auto last_error = GetLastError();
			qCritical() << "Failed to unload library (last error: " << last_error << ")";
		}
#else
		Q_UNUSED(library_module);
#endif
	}

	QFunctionPointer resolve_library_symbol(LibraryModule library_module, const QString& symbol_name)
	{
#if defined(Q_OS_WINDOWS)
		return QFunctionPointer(GetProcAddress(library_module, symbol_name.toUtf8()));
#else
		Q_UNUSED(library_module);
		Q_UNUSED(symbol_name);
		return nullptr;
#endif
	}
}

namespace VadonEditor::Core
{
	struct PluginManager::Internal
	{
		Application& m_application;

		QHash<PluginHandle, PluginData> m_plugins;
		PluginHandle m_plugin_counter = 0;

		Internal(Application& application)
			: m_application(application)
		{

		}

		bool initialize()
		{
			// TODO: anything?
			return true;
		}

		void shutdown()
		{
			// TODO: unload any leftover active plugins!
		}

		PluginHandle load_plugin(const PluginInfo& info)
		{
			for (auto plugin_it = m_plugins.begin(); plugin_it != m_plugins.end(); ++plugin_it)
			{
				if (plugin_it->info == info)
				{
					return plugin_it.key();
				}
			}

			PluginData new_plugin_data;
			new_plugin_data.info = info;
			new_plugin_data.library_module = load_plugin_library(info.path);

			if (new_plugin_data.library_module == 0)
			{
				qCritical() << "Failed to load library";
				return 0;
			}
			qDebug() << "Loaded library at \"" << info.path << "\"";

			++m_plugin_counter;
			const PluginHandle new_plugin_handle = m_plugin_counter;

			m_plugins.insert(new_plugin_handle, new_plugin_data);

			return new_plugin_handle;
		}

		void unload_plugin(PluginHandle plugin_handle)
		{
			auto plugin_it = m_plugins.find(plugin_handle);
			if(plugin_it == m_plugins.end())
			{
				qCritical() << "Plugin not loaded!";
				return;
			}

			unload_plugin_library(plugin_it->library_module);
			plugin_it->library_module = 0;

			m_plugins.erase(plugin_it);
		}

		QFunctionPointer get_plugin_function(PluginHandle plugin_handle, const QString& name)
		{
			auto plugin_it = m_plugins.find(plugin_handle);
			if (plugin_it == m_plugins.end())
			{
				qCritical() << "Plugin not loaded!";
				return 0;
			}

			QFunctionPointer function_pointer = resolve_library_symbol(plugin_it->library_module, name);
			if (function_pointer == nullptr)
			{
				qCritical() << "Function \"" << name << "\" not found in plugin!";
				return 0;
			}

			return function_pointer;
		}
	};

	PluginManager::~PluginManager() = default;

	PluginHandle PluginManager::load_plugin(const PluginInfo& info)
	{
		return m_internal->load_plugin(info);
	}

	void PluginManager::unload_plugin(PluginHandle plugin_handle)
	{
		m_internal->unload_plugin(plugin_handle);
	}

	QFunctionPointer PluginManager::get_plugin_function(PluginHandle plugin_handle, const QString& name)
	{
		return m_internal->get_plugin_function(plugin_handle, name);
	}

	PluginManager::PluginManager(Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{

	}

	bool PluginManager::initialize()
	{
		return m_internal->initialize();
	}

	void PluginManager::shutdown()
	{
		m_internal->shutdown();
	}
}