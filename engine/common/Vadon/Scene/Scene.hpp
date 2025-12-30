#ifndef VADON_SCENE_SCENE_HPP
#define VADON_SCENE_SCENE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
namespace Vadon::Scene
{
	struct Scene;

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Scene, SceneID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Scene, SceneHandle);

	// FIXME: remove this once we extract serialization/editing from core systems
	struct SceneComponent
	{
		SceneID parent_scene; // Indicates that this Entity was instantiated as part of another scene
		SceneID root_scene; // Indicates that this Entity is the root of an instantiated scene
		// FIXME: should we use "owner", similar to Godot?
		// TODO: flags and other metadata (e.g indicate that we override certain elements)
	};
}

VADON_REGISTER_TYPE_UUID(Vadon::Scene::Scene, "e9918681-2828-4d0c-8037-f8a44ca29eac");
VADON_REGISTER_TYPE_UUID(Vadon::Scene::SceneComponent, "348aa1b3-3bce-4002-a5f2-440473dd0228");
#endif