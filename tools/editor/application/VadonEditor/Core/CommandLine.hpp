#ifndef VADONEDITOR_CORE_COMMANDLINE_HPP
#define VADONEDITOR_CORE_COMMANDLINE_HPP
#include <QString>
namespace VadonEditor::Core
{
	enum class CommandLineParameter
	{
		IS_SIMULATOR,
		IS_SCHEMA_EXPORTER,
		STARTUP_PROJECT_PATH,
		DEBUG_BREAK_ON_INIT
	};

	struct CommandLineState
	{
		bool is_simulator = false;
		bool is_schema_exporter = false;
		QString startup_project_path;
		bool debug_break_on_init = false;

		static constexpr const char* get_parameter_key(CommandLineParameter parameter)
		{
			switch (parameter)
			{
			case CommandLineParameter::IS_SIMULATOR:
				return "simulator";
			case CommandLineParameter::IS_SCHEMA_EXPORTER:
				return "schema-exporter";
			case CommandLineParameter::STARTUP_PROJECT_PATH:
				return "startup-project";
			case CommandLineParameter::DEBUG_BREAK_ON_INIT:
				return "debug-break-on-init";
			}

			return "";
		}
	};
}
#endif