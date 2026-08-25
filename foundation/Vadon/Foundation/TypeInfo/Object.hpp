#ifndef VADON_FOUNDATION_TYPEINFO_OBJECT_HPP
#define VADON_FOUNDATION_TYPEINFO_OBJECT_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		// NOTE: Foundation only defines the data structures and the data keys to use
		// Serialization is delegated to client code
		struct DataObjectSchema
		{
			static constexpr PropertySchema c_type_property{
				UUIDString{"de0a777b-505a-42ce-b620-8ee4a9283be5"},
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_properties_property{
				UUIDString{"bb8e2650-dca2-4700-b539-02c5d7a00bc7"},
				TypeInfo::get_base_type_uuid_string(BaseType::DICTIONARY)
			};

			static constexpr UUIDString c_type_uuid{ "0906819c-304e-4176-9613-dccce94ef50a" };

			// NOTE: this is for when 
			enum PathSerializerKey
			{
				TRIVIAL,
				ARRAY,
				DICTIONARY,
				OBJECT,
				KEY_COUNT
			};

			// FIXME: use UUIDs instead?
			static constexpr const char* c_path_serializer_key_strings[static_cast<uint32>(PathSerializerKey::KEY_COUNT)] = {
				"value",
				"array",
				"dictionary",
				"object"
			};

			static constexpr const char* get_path_serializer_key(PathSerializerKey key)
			{
				return c_path_serializer_key_strings[static_cast<uint32>(key)];
			}
		};

		struct ObjectWrapperSchema
		{
			static constexpr UUIDString c_type_uuid{ "e5159621-3d6b-4964-87b8-fe66041471ef" };
		};
	}
}
#endif