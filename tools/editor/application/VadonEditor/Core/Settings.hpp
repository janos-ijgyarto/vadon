#ifndef VADONEDITOR_CORE_SETTINGS_HPP
#define VADONEDITOR_CORE_SETTINGS_HPP
#include <QString>
namespace VadonEditor::Core
{
	// TODO: split into categories!
	struct EditorSettings
	{
		bool is_simulator = false;
		QString plugin_path;
	};
}
#endif