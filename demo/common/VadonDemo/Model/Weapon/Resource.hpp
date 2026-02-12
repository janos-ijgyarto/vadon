#ifndef VADONDEMO_MODEL_WEAPON_RESOURCE_HPP
#define VADONDEMO_MODEL_WEAPON_RESOURCE_HPP
#include <Vadon/Scene/Scene.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::Model
{
	struct WeaponDefinition : public Vadon::Scene::Resource
	{
		VADON_DECLARE_MEMBER_UUID(projectile_prefab, "db7ffb25-cc57-44f4-9a4c-d40fc26dd3c4");
		VADON_DECLARE_MEMBER_UUID(rate_of_fire, "31f2bac3-bfad-42a4-9589-0d8a4f2f92ca");

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		Vadon::Scene::SceneID projectile_prefab;
		float rate_of_fire = 1.0f;

		static void register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(WeaponDefinition, WeaponDefID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(WeaponDefinition, WeaponDefHandle);
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::WeaponDefinition, "9e95dcb8-ae6e-400f-823f-a46f7640f5af");
#endif