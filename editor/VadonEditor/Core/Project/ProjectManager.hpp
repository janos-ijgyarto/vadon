#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/Core/Module.hpp>
#include <VadonEditor/Core/Project/Project.hpp>
#include <VadonEditor/Core/Project/Asset/AssetLibrary.hpp>
#include <functional>
namespace VadonEditor::Core
{
	class ProjectManager : public CoreSystem<ProjectManager>
	{
	public:
		enum class State
		{
			LAUNCHER,
			PROJECT_OPEN,
			PROJECT_LOADED,
			PROJECT_CLOSED
		};

		State get_state() const { return m_state; }

		const ProjectInfoList& get_project_cache() const { return m_project_cache; }
		
		const Project& get_active_project() const { return m_active_project; }

		// NOTE: client code is expected to set this as required by the project
		void set_project_custom_properties(const Vadon::Utilities::PropertyList& properties) { m_custom_properties = properties; }
		void update_project_custom_properties(const Vadon::Utilities::PropertyList& properties);

		// FIXME: implement some other way than having to do it via callbacks?
		using ProjectPropertiesCallback = std::function<void(const Project&)>;
		VADONEDITOR_API void register_project_properties_callback(ProjectPropertiesCallback callback);

		bool create_project(std::string_view project_name, std::string_view root_path);
		bool open_project(std::string_view path);
		VADONEDITOR_API bool load_project_data();
		bool export_project(std::string_view output_path);
		void close_project();

		AssetLibrary& get_asset_library() { return m_asset_library; }
	private:
		ProjectManager(Editor& editor);

		bool initialize();
		void load_project_cache();
		bool open_startup_project();

		bool load_project_metadata(std::string_view root_path);
		void add_project_to_cache(const ProjectInfo& project);

		bool internal_save_project_file(Vadon::Core::Project& project_info);

		State m_state;
		ProjectInfoList m_project_cache;
		Project m_active_project;

		Vadon::Utilities::PropertyList m_custom_properties;
		std::vector<ProjectPropertiesCallback> m_properties_callbacks;

		AssetLibrary m_asset_library;

		friend Editor;
	};
}
#endif