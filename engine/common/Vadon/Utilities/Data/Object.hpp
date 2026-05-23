#ifndef VADON_UTILITIES_DATA_OBJECT_HPP
#define VADON_UTILITIES_DATA_OBJECT_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon::Utilities
{
	class Serializer;

	using ObjectPointer = void*;

	class DataObject
	{
	public:
		static Vadon::Foundation::UUID deserialize_type_uuid(Serializer& serializer);

		static bool serialize_object(Serializer& serializer, const Vadon::Foundation::UUID& type_uuid, ObjectPointer object_ptr);
		static bool serialize_object_properties(Serializer& serializer, const Vadon::Foundation::UUID& type_uuid, ObjectPointer object_ptr);
	};
}
#endif