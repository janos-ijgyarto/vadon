#ifndef VADONEDITOR_CORE_PLUGIN_PLUGIN_HPP
#define VADONEDITOR_CORE_PLUGIN_PLUGIN_HPP
#include <QString>
namespace VadonEditor::Core
{
	struct PluginInfo
	{
		QString path;
		// TODO: any other info?

		bool operator==(const PluginInfo& other) const { return path == other.path; }
	};
}
#endif