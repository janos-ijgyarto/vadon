#include <VadonDemo/Core/Configuration.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Core
{
	void GlobalConfiguration::register_type(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<GlobalConfiguration, Vadon::Model::Resource>();

		TypeRegistry::add_property<GlobalConfiguration>(VADON_GET_MEMBER_UUID(GlobalConfiguration, main_menu_scene), Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::main_menu_scene>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<GlobalConfiguration>(VADON_GET_MEMBER_UUID(GlobalConfiguration, default_start_level), Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::default_start_level>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<GlobalConfiguration>(VADON_GET_MEMBER_UUID(GlobalConfiguration, viewport_size), Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::viewport_size>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata global_config_metadata(metadata_registry, VADON_GET_TYPE_UUID(GlobalConfiguration));
		global_config_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Core::GlobalConfiguration");

		global_config_metadata.add_property(VADON_GET_MEMBER_UUID(GlobalConfiguration, main_menu_scene))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Main Menu Scene");

		global_config_metadata.add_property(VADON_GET_MEMBER_UUID(GlobalConfiguration, default_start_level))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Default Start Level");

		global_config_metadata.add_property(VADON_GET_MEMBER_UUID(GlobalConfiguration, viewport_size))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Viewport Size");
	}
}