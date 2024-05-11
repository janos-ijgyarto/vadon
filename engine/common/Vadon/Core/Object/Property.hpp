#ifndef VADON_CORE_OBJECT_PROPERTY_HPP
#define VADON_CORE_OBJECT_PROPERTY_HPP
#include <Vadon/Core/Object/Variant.hpp>
namespace Vadon::Core
{
	struct ObjectPropertyInfo
	{
		std::string name;
		size_t type_index;
		std::string getter;
		std::string setter;
	};

	// TODO: make this a class so we can use builder pattern to add properties?
	using ObjectPropertyInfoList = std::vector<ObjectPropertyInfo>;

	struct ObjectProperty
	{
		std::string name;
		Variant value;
	};

	using ObjectPropertyList = std::vector<ObjectProperty>;
}
#define VADON_OBJECT_PROPERTY(_name, _property_type, _getter, _setter) Vadon::Core::ObjectPropertyInfo{ .name = _name, .type_index = Vadon::Core::type_list_index_v<_property_type, Vadon::Core::Variant>, .getter = _getter, .setter = _setter }
#define VADON_ADD_OBJECT_PROPERTY(_name, _property_type, _getter, _setter) std::move(VADON_OBJECT_PROPERTY(_name, _property_type, _getter, _setter))
#define VADON_ADD_OBJECT_MEMBER_PROPERTY(_name, _member, _getter, _setter) std::move(VADON_OBJECT_PROPERTY(_name, decltype(_member), _getter, _setter))
#endif