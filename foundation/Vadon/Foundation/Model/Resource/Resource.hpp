#ifndef VADON_FOUNDATION_MODEL_RESOURCE_RESOURCE_HPP
#define VADON_FOUNDATION_MODEL_RESOURCE_RESOURCE_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		// NOTE: Foundation only defines the data structures and the data keys to use
		// Serialization is delegated to client code
		struct ResourceSchema
		{
			static constexpr UUIDString c_type_uuid{ "e9918681-2828-4d0c-8037-f8a44ca29eac" };

			static constexpr PropertySchema c_id_property{
				UUIDString{"190cfdfc-a70c-4723-b640-3975d7b675fc"},
				get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_type_property{
				UUIDString{"19565dac-0540-48a8-b5e5-c692c85d9318"},
				get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_properties_property{
				UUIDString{"08549120-323c-48ee-9086-f5bf52c435e2"},
				get_base_type_uuid_string(BaseType::DICTIONARY)
			};

			static constexpr PropertySchema c_embedded_property{
				UUIDString{"512029ab-4cbc-4ae7-8996-1b520ebf8951"},
				get_base_type_uuid_string(BaseType::ARRAY)
			};
		};
	}
}
#endif