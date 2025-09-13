#include <VadonDemo/Model/Collision/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void Collision::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Collision>();

		Vadon::Utilities::TypeRegistry::add_property<Collision>("radius", Vadon::Utilities::MemberVariableBind<&Collision::radius>().bind_member_getter().bind_member_setter());
	}
}