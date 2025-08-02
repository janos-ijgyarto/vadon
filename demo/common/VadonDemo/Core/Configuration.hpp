#ifndef VADONDEMO_CORE_CONFIGURATION_HPP
#define VADONDEMO_CORE_CONFIGURATION_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonDemo::Core
{
	// TODO: can also combine with ECS
	// Config can point to a "core entity scene", and the scene has entities with components
	// which contain config data for subsystems. Can use event logic to update subsystems when config changes
	struct GlobalConfiguration
	{
		Vadon::Scene::SceneID main_menu_scene;
		Vadon::Scene::SceneID default_start_level;
		Vadon::Math::Vector2 viewport_size = { 1024, 768 };

		VADONDEMO_API static void register_type();
		VADONDEMO_API static Vadon::Utilities::PropertyList get_default_properties();
	};
}
#endif