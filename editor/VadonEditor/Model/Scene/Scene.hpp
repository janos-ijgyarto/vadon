#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <Vadon/Scene/Scene.hpp>
namespace VadonEditor::Model
{
	struct Scene
	{
		Vadon::Scene::ResourceID scene_id;
		Vadon::Scene::ResourcePath path;
	};

	using SceneList = std::vector<Scene>;
}
#endif