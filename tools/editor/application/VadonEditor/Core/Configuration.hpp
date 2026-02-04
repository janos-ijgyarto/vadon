#ifndef VADONEDITOR_CORE_CONFIGURATION_HPP
#define VADONEDITOR_CORE_CONFIGURATION_HPP
#include <QString>
namespace VadonEditor::Core
{
	enum class ApplicationMode
	{
		EDITOR,
		SIMULATOR,
		SCHEMA_EXPORTER
	};

	struct Configuration
	{
		ApplicationMode mode = ApplicationMode::EDITOR;
		QString startup_project_path;
		// TODO: other settings?
	};
}
#endif