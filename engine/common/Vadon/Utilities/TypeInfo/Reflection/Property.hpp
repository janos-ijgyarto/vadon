#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Utilities
{
	using PropertyUUID = ::Vadon::Foundation::UUID;

	struct PropertyInfo
	{
		::Vadon::Foundation::Property base_info;
		bool has_getter = false;
		bool has_setter = false;
		// FIXME: any other metadata?
	};

	// TODO: make this a class so we can use builder pattern to add properties?
	using PropertyInfoList = std::vector<PropertyInfo>;

	struct Property
	{
		::Vadon::Foundation::Property info;
		Variant value;
	};

	using PropertyList = std::vector<Property>;
}

#endif