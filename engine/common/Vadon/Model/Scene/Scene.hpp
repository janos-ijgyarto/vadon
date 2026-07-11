#ifndef VADON_MODEL_SCENE_SCENE_HPP
#define VADON_MODEL_SCENE_SCENE_HPP
#include <Vadon/Foundation/Model/Scene/Scene.hpp>
#include <Vadon/Model/Resource/Resource.hpp>
namespace Vadon::Model
{
	struct Scene;

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(Scene, SceneID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(Scene, SceneHandle);

	struct EntitySceneInfo
	{
		SceneID scene_id; // ID of the scene this Entity is currently a part of
		::Vadon::Foundation::UUID entity_id; // ID of the Entity within the scene data
	};

	struct SceneComponent
	{
		EntitySceneInfo scene_info; // Info for the Scene that defines this Entity
		EntitySceneInfo parent_scene_info; // If Entity is sub-scene, this has info for the Scene that this is a sub-scene of
	};
}

VADON_REGISTER_TYPE_UUID(Vadon::Model::Scene, ::Vadon::Foundation::SceneSchema::c_type_uuid.string );
VADON_REGISTER_TYPE_UUID(Vadon::Model::SceneComponent, "348aa1b3-3bce-4002-a5f2-440473dd0228");
#endif