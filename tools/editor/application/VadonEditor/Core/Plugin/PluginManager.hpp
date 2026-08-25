#ifndef VADONEDITOR_CORE_PLUGIN_PLUGINMANAGER_HPP
#define VADONEDITOR_CORE_PLUGIN_PLUGINMANAGER_HPP
#include <memory>
#include <QFunctionPointer>
class QString;
namespace VadonEditor::Core
{
	struct PluginInfo;

	using PluginHandle = uint32_t;

	class Application;

	class PluginManager
	{
	public:
		static constexpr PluginHandle c_invalid_plugin_handle = PluginHandle(0);

		~PluginManager();

		PluginHandle load_plugin(const PluginInfo& info);
		void unload_plugin(PluginHandle plugin_handle);

		QFunctionPointer get_plugin_function(PluginHandle plugin_handle, const QString& name);
	private:
		PluginManager(Application& application);

		bool initialize();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Application;
	};
}
// TODO: move to general utility macro header?
#define VADONEDITOR_API_FUNCTION_POINTER(_name) decltype(&(_name))
#define VADONEDITOR_API_FUNCTION_NAME(_name) #_name
#endif