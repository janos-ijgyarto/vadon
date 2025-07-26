#ifndef VADONDEMO_MODEL_RESOURCE_HPP
#define VADONDEMO_MODEL_RESOURCE_HPP
#include <Vadon/Scene/Scene.hpp>
namespace VadonDemo::Model
{
	struct WeaponDefinition : public Vadon::Scene::Resource
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		Vadon::Scene::SceneID projectile_prefab;
		float rate_of_fire = 1.0f;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(WeaponDefinition, WeaponDefID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(WeaponDefinition, WeaponDefHandle);
}
#endif