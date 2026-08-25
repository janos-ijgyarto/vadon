#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <QList>
#include <QString>
#include <QUuid>
namespace VadonEditor::Core
{
	struct EditorPluginInfo
	{
		QString path;
		QString configuration_name;
	};

	struct EditorPluginSettings
	{
		QString custom_search_path;
		QString selected_configuration;
	};

	struct GameExecutableInfo
	{
		QString path;
		QString configuration_name;
	};

	struct GameExecutableSettings
	{
		QString custom_search_path;
		QString selected_configuration;
	};

	struct ProjectInfo
	{
		static constexpr const char* c_project_file_name = "project.vdpr";

		QString name;
		QString root_path;

		EditorPluginSettings plugin_settings;
		QList<EditorPluginInfo> plugin_entries;

		GameExecutableSettings game_settings;
		QList<GameExecutableInfo> game_entries;

		QUuid custom_data_resource_id;
		// TODO: anything else?

		QString get_project_file_path() const
		{
			return QString("%1/%2").arg(root_path).arg(c_project_file_name);
		}

		const EditorPluginInfo* find_plugin_entry(const QString& config_name) const
		{
			for (const EditorPluginInfo& current_entry : plugin_entries)
			{
				if (current_entry.configuration_name == config_name)
				{
					return &current_entry;
				}
			}

			return nullptr;
		}

		const GameExecutableInfo* find_game_entry(const QString& config_name) const
		{
			for (const GameExecutableInfo& current_entry : game_entries)
			{
				if (current_entry.configuration_name == config_name)
				{
					return &current_entry;
				}
			}

			return nullptr;
		}
	};
}
#endif