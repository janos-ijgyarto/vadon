#ifndef VADONEDITOR_CORE_COMMANDLINE_HPP
#define VADONEDITOR_CORE_COMMANDLINE_HPP
#include <QString>
namespace VadonEditor::Core
{
	enum class CommandLineParameter
	{
		IS_ASSET_SERVER,
		IS_SIMULATOR,
		IS_SCHEMA_EXPORTER,
		STARTUP_PROJECT_PATH,
		PLUGIN_CONFIG_NAME,
		DEBUG_BREAK_ON_INIT
	};

	struct CommandLineState
	{
		bool is_asset_server = false;
		bool is_simulator = false;
		bool is_schema_exporter = false;
		QString startup_project_path;
		QString plugin_config_name;
		bool debug_break_on_init = false;

		static constexpr const char* get_parameter_key(CommandLineParameter parameter)
		{
			switch (parameter)
			{
			case CommandLineParameter::IS_ASSET_SERVER:
				return "asset-server";
			case CommandLineParameter::IS_SIMULATOR:
				return "simulator";
			case CommandLineParameter::IS_SCHEMA_EXPORTER:
				return "schema-exporter";
			case CommandLineParameter::STARTUP_PROJECT_PATH:
				return "startup-project";
			case CommandLineParameter::PLUGIN_CONFIG_NAME:
				return "plugin-config-name";
			case CommandLineParameter::DEBUG_BREAK_ON_INIT:
				return "debug-break-on-init";
			}

			return "";
		}
	};
}
#endif