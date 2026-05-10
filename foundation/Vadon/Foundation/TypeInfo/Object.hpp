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
				get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_properties_property{
				UUIDString{"bb8e2650-dca2-4700-b539-02c5d7a00bc7"},
				get_base_type_uuid_string(BaseType::DICTIONARY)
			};
		};
	}
}
#endif