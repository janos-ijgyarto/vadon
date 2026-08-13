#ifndef VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#define VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <QHash>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class SceneSystem
	{
	public:
		~SceneSystem();

		// NOTE: if a scene object exists, that implies its data was already loaded
		// TODO: use refcount system to ensure resource can't be unloaded while
		// Scene object still exists!
		Scene* create_scene();
		Scene* get_scene(const SceneID& scene_id);
		void remove_scene(Scene* scene);

		int create_scene_asset(const SceneID& scene_id, const QString& path);
		int find_scene_asset(const SceneID& scene_id) const;

		QList<Scene*> get_all_scenes() const { return m_scene_lookup.values(); }
		void save_all_scenes();

		void generate_temp_scenes();
	private:
		SceneSystem(Core::Application& application);

		bool initialize();
		void project_loaded();
		void shutdown();

		void simulator_initialized();

		Scene* internal_add_new_scene(Resource* scene_resource);

		Core::Application& m_application;
		QHash<SceneID, Scene*> m_scene_lookup;

		friend class ModelSystem;
	};
}
#endif