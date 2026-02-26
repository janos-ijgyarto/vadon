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
	private:
		SceneSystem(Core::Application& application);

		bool initialize();

		Core::Application& m_application;
		QHash<SceneID, Scene> m_scene_lookup;

		friend class ModelSystem;
	};
}
#endif