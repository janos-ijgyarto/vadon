#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#include <Vadon/Utilities/TypeInfo/ErasedDataType.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Utilities
{
	struct PropertyInfo
	{
		std::string name;
		ErasedDataTypeID data_type;
		// FIXME: any other metadata?
		bool has_getter = false;
		bool has_setter = false;
	};

	// TODO: make this a class so we can use builder pattern to add properties?
	using PropertyInfoList = std::vector<PropertyInfo>;

	struct Property
	{
		std::string name;
		ErasedDataTypeID data_type;
		Variant value;
	};

	using PropertyList = std::vector<Property>;
}

#endif