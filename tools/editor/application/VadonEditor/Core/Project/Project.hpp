#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <QString>
#include <QUuid>
namespace VadonEditor::Core
{
	struct ProjectInfo
	{
		static constexpr const char* c_project_file_name = "project.vdpr";

		QString name;
		QString root_path;
		QString plugin_path;
		QUuid custom_data_id;
		// TODO: anything else?

		QString get_project_file_path() const
		{
			return QString("%1/%2").arg(root_path).arg(c_project_file_name);
		}
	};
}
#endif