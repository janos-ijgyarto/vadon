#include <VadonDemo/Model/Collision/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void Collision::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Collision>();

		Vadon::Utilities::TypeRegistry::add_property<Collision>(VADON_GET_MEMBER_UUID(Collision, radius), Vadon::Utilities::MemberVariableBind<&Collision::radius>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Collision))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Collision")
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model")
			.add_property(VADON_GET_MEMBER_UUID(Collision, radius))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Radius")
				.commit_property();
	}
}