#ifndef VADON_FOUNDATION_TYPEINFO_TYPEINFO_HPP
#define VADON_FOUNDATION_TYPEINFO_TYPEINFO_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon
{
	namespace Foundation
	{
		// FIXME: use actual type list instead of enum to create more explicit link?
		enum class BaseType : uint32
		{
			INVALID,
			INT32,
			UINT32,
			FLOAT,
			BOOL,
			STRING,
			VECTOR2,
			VECTOR2I,
			VECTOR3,
			VECTOR3I,
			VECTOR4,
			COLORRGBA,
			UUID,
			ARRAY, // TODO: add typed version!
			DICTIONARY, // TODO: add typed version!
			TYPE_COUNT
		};

		struct TypeInfo
		{
			UUID id;
			UUID base_id;
			size_t size;
			size_t alignment;
			unsigned int property_count;
		};

		constexpr UUIDString c_base_type_uuids[] = {
			"INVALID",
			"9b2d1939-5e7e-4853-907c-de197c53289f",
			"b3e7681a-6ed6-4ea2-8214-95fd11fe1d51",
			"4ffacbf5-8dfc-4d83-9c95-f78b8e1ab220",
			"6ad52077-ce8f-436d-af3c-7bcb4aaf601e",
			"984ce8f7-3d06-4331-81b0-a5682e9b1254",
			"fe2955e7-013c-4a9f-9c6a-ac23fb7e9065",
			"594ba83b-a278-41ca-bb89-9c494a44979b",
			"592dfbad-4c87-4d4b-9c3d-a7fb2d1d52ab",
			"90c473e1-aea7-4858-9626-bc69ea8ecff4",
			"01a872bd-dc48-4fb4-ac5a-8e96e53ed949",
			"b40352b7-a861-482e-b614-af090e88db70",
			"007d2133-22db-4e0a-aca8-300314fb87da",
			"295c9fea-3d4b-4571-80e1-a04ac588bf7e",
			"5ef32604-3246-4a76-a96f-5bb0eb2ba0cc"
		};

		constexpr UUIDString get_base_type_uuid_string(BaseType base_type)
		{
			return c_base_type_uuids[static_cast<uint32>(base_type)];
		}
	}
}
#endif