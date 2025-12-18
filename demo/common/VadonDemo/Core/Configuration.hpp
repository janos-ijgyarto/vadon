#ifndef VADONDEMO_CORE_CONFIGURATION_HPP
#define VADONDEMO_CORE_CONFIGURATION_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonDemo::Core
{
	// This is a resource that we will reference in the project file
	struct GlobalConfiguration : Vadon::Scene::Resource
	{
		Vadon::Scene::SceneID main_menu_scene;
		Vadon::Scene::SceneID default_start_level;
		Vadon::Math::Vector2 viewport_size = { 1024, 768 };

		VADONDEMO_API static void register_type();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(GlobalConfiguration, GlobalConfigurationID, GlobalConfigurationHandle);
}
#endif