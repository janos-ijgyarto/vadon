#ifndef VADONEDITOR_CORE_COMMANDLINE_HPP
#define VADONEDITOR_CORE_COMMANDLINE_HPP
#include <QUuid>
namespace VadonEditor::Core
{
	// TODO: split into categories!
	struct CommandLineParameters
	{
		bool is_simulator = false;
		bool debug_break_on_init = false;
		QString startup_project_path;
	};
}
#endif