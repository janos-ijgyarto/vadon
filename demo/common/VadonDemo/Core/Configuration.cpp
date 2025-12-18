#include <VadonDemo/Core/Configuration.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Core
{
	void GlobalConfiguration::register_type()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<GlobalConfiguration, Vadon::Scene::Resource>();

		TypeRegistry::add_property<GlobalConfiguration>("main_menu_scene", Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::main_menu_scene>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<GlobalConfiguration>("default_start_level", Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::default_start_level>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<GlobalConfiguration>("viewport_size", Vadon::Utilities::MemberVariableBind<&GlobalConfiguration::viewport_size>().bind_member_getter().bind_member_setter());
	}
}