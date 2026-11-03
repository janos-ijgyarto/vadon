#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <QDir>
#include <QList>
#include <QUuid>
namespace VadonEditor::Core
{
	// NOTE: source project refers to the editor-agnostic project data that
	// the engine can parse. The editor can modify it, but it does not
	// add any editor-specific data
	struct SourceProjectInfo
	{
		static constexpr const char* c_project_file_name = "project.vdpr";

		QString name;
		QString root_path;

		bool is_valid() const
		{
			return (name.isEmpty() == false) && (root_path.isEmpty() == false);
		}

		static QString get_project_file_path(const QString& project_root_path)
		{
			return QDir::cleanPath(project_root_path + "/" + c_project_file_name);
		}

		QString get_project_file_path() const
		{
			return get_project_file_path(root_path);
		}
	};

	struct SourceProject
	{
		SourceProjectInfo info;
		QUuid custom_data_resource_id;

		bool is_valid() const
		{
			return info.is_valid();
		}
	};

	// NOTE: Editor project contains all relevant metadata for editing and exporting the project
	// Equivalent to the project file used by an image editor to produce image raw data
	struct EditorProjectInfo
	{
		static constexpr const char* c_file_name = "project.vdepr";

		QString source_path;
		QString output_path;

		bool is_valid() const
		{
			return (source_path.isEmpty() == false) && (output_path.isEmpty() == false);
		}

		QString get_source_project_file_path() const { return SourceProjectInfo::get_project_file_path(source_path); }

		QString get_project_file_path() const
		{
			return QDir::cleanPath(output_path + "/" + c_file_name);
		}
	};

	struct EditorPluginInfo
	{
		QString path;
		QString configuration_name;
	};

	struct EditorPluginSettings
	{
		QString binaries_path;
		QString selected_configuration;
	};

	struct GameExecutableInfo
	{
		QString path;
		QString configuration_name;
	};

	struct GameExecutableSettings
	{
		QString binaries_path;
		QString selected_configuration;
	};

	struct EditorProject
	{
		EditorProjectInfo info;

		EditorPluginSettings plugin_settings;
		QList<EditorPluginInfo> plugin_entries;

		GameExecutableSettings game_settings;
		QList<GameExecutableInfo> game_entries;

		bool is_valid() const { return info.is_valid(); }

		const QString& get_plugin_binaries_path() const { return plugin_settings.binaries_path.isEmpty() ? info.output_path : plugin_settings.binaries_path; }

		const QString& get_game_binaries_path() const { return game_settings.binaries_path.isEmpty() ? info.output_path : game_settings.binaries_path; }

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