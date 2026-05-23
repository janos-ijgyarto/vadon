#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTY_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Utilities
{
	using PropertyUUID = ::Vadon::Foundation::UUID;

	enum class PropertyCategory
	{
		TRIVIAL,
		ARRAY,
		OBJECT
	};

	struct PropertyInfo
	{
		::Vadon::Foundation::Property base_info;
		TypeID data_type;
		bool has_getter = false;
		bool has_setter = false;
		// FIXME: any other metadata?

		PropertyCategory get_category() const;
	};

	// TODO: make this a class so we can use builder pattern to add properties?
	using PropertyInfoList = std::vector<PropertyInfo>;

	struct Property
	{
		::Vadon::Foundation::Property info;
		Variant value;

		static constexpr ::Vadon::Foundation::UUID property_schema_to_uuid(const ::Vadon::Foundation::PropertySchema& schema)
		{
			return string_to_uuid(schema.id);
		}
	};

	using PropertyList = std::vector<Property>;
}

#endif