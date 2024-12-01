#ifndef VADON_SCENE_SCENE_HPP
#define VADON_SCENE_SCENE_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
namespace Vadon::Scene
{
	struct Scene : public ResourceBase
	{
	};

	VADON_DECLARE_TYPED_RESOURCE_HANDLE(Scene, SceneHandle);

	struct SceneComponent
	{
		SceneHandle parent_scene; // Indicates that this Entity was instantiated as part of another scene
		SceneHandle root_scene; // Indicates that this Entity is the root of an instantiated scene
		// FIXME: should we use "owner", similar to Godot?
		// TODO: flags and other metadata (e.g indicate that we override certain elements)
	};
}
#endif