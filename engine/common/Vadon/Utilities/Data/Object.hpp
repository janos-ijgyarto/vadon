#ifndef VADON_UTILITIES_DATA_OBJECT_HPP
#define VADON_UTILITIES_DATA_OBJECT_HPP
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	struct ObjectVariantTypeTrait
	{
		static Variant to_variant(const T& value)
		{
			return ObjectPointer{ .type = TypeRegistry::get_type_id<T>(), .data = const_cast<T*>(&value) };
		}

		static T from_variant(const Variant& variant)
		{
			return *static_cast<T*>(std::get<ObjectPointer>(variant).data);
		}
	};

	class Serializer;

	class DataObject
	{
	public:
		static Vadon::Foundation::UUID deserialize_type_uuid(Serializer& serializer);

		VADONCOMMON_API static bool serialize_object(Serializer& serializer, ObjectPointer& object);
		VADONCOMMON_API static bool serialize_object_properties(Serializer& serializer, ObjectPointer& object);
	};
}
#define VADON_DEFINE_OBJECT_VARIANT_TYPE_TRAIT(_type) template<>\
struct Vadon::Utilities::VariantTypeTrait<_type>\
{\
	static Variant to_variant(const _type& value)\
	{\
		return ObjectVariantTypeTrait<_type>::to_variant(value);\
	}\
	static _type from_variant(const Variant& variant)\
	{\
		return ObjectVariantTypeTrait<_type>::from_variant(variant);\
	}\
}
#endif