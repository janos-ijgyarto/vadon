#ifndef VADON_UTILITIES_TYPEINFO_PROPERTY_HPP
#define VADON_UTILITIES_TYPEINFO_PROPERTY_HPP
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Utilities
{
	struct PropertyInfo
	{
		std::string name;
		size_t type_index;
		// FIXME: any other metadata?
		bool has_getter = false;
		bool has_setter = false;
	};

	// TODO: make this a class so we can use builder pattern to add properties?
	using PropertyInfoList = std::vector<PropertyInfo>;

	struct Property
	{
		std::string name;
		Variant value;
	};

	using PropertyList = std::vector<Property>;
}

#endif