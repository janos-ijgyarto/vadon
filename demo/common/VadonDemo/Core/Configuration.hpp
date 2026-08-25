#ifndef VADONDEMO_CORE_CONFIGURATION_HPP
#define VADONDEMO_CORE_CONFIGURATION_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Model/Scene/Scene.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::Core
{
	// This is a resource that we will reference in the project file
	struct GlobalConfiguration : Vadon::Model::Resource
	{
		VADON_DECLARE_MEMBER_UUID(main_menu_scene, "d15974ef-74d8-40f2-99e8-bf9e00db55f9");
		VADON_DECLARE_MEMBER_UUID(default_start_level, "5a8cbf20-d671-453c-8e89-42cbcf402b71");
		VADON_DECLARE_MEMBER_UUID(viewport_size, "6e4b1dc8-73df-4b82-9783-e4019af94f84");

		Vadon::Model::SceneID main_menu_scene;
		Vadon::Model::SceneID default_start_level;
		Vadon::Math::Vector2 viewport_size = { 1024, 768 };

		VADONDEMO_API static void register_type(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	// FIXME: use pointer-to-member trait 

	VADON_MODEL_DECLARE_TYPED_RESOURCE_REFERENCES(GlobalConfiguration, GlobalConfigurationID, GlobalConfigurationHandle);
}
VADON_REGISTER_TYPE_UUID(VadonDemo::Core::GlobalConfiguration, "a796a83b-6b81-40b5-8d6e-7c5497dc2ac8");
#endif