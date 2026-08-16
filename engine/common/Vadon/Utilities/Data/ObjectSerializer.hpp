#ifndef VADON_UTILITIES_DATA_OBJECTSERIALIZER_HPP
#define VADON_UTILITIES_DATA_OBJECTSERIALIZER_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>

#include <span>
namespace Vadon::Utilities
{
	struct PropertyPathElement
	{
		size_t index = 0;
		::Vadon::Foundation::UUID uuid;

		bool is_object_key() const { return uuid.is_valid() == true; }
		bool is_array_index() const { return is_object_key() == false; }
	};

	using PropertyPath = std::span<PropertyPathElement>;

	struct PropertyInfo;

	class ObjectWrapper;
	class Serializer;

	class ObjectSerializer
	{
	public:
		VADONCOMMON_API static bool serialize_object(Serializer& serializer, VariantDictionary& object_dictionary);
		VADONCOMMON_API static bool serialize_object_properties(Serializer& serializer, TypeID object_type, VariantDictionary& object_properties);

		VADONCOMMON_API static bool load_object_data(ObjectWrapper& object, const VariantDictionary& data);
		VADONCOMMON_API static bool load_object_property_data(ObjectWrapper& object, const VariantDictionary& property_data);

		VADONCOMMON_API static bool store_object_data(const ObjectWrapper& object, VariantDictionary& data);
		VADONCOMMON_API static bool store_object_property_data(const ObjectWrapper& object, VariantDictionary& property_data);

		// NOTE: this is expected to be used with serializing specific properties, possibly deep within
		// the data hierarchy of an object. The client will need to gather the necessary property metadata
		// and pass the data in/out accordingly
		VADONCOMMON_API static bool serialize_property_data(Serializer& serializer, const PropertyInfo& property_info, Variant& property_data);
	};
}
#endif