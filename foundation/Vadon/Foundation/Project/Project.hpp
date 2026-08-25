#ifndef VADON_FOUNDATION_PROJECT_PROJECT_HPP
#define VADON_FOUNDATION_PROJECT_PROJECT_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct ProjectInfoSchema
		{
			static constexpr PropertySchema c_name_property{
				UUIDString{"67eb5876-e8c9-47e5-b512-6a3d52ce1d0d"},
				TypeInfo::get_base_type_uuid_string(BaseType::STRING)
			};

			static constexpr PropertySchema c_custom_data_resource_property{
				UUIDString{"327fccb8-726d-4e25-9270-6c9060d7c8c7"},
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			// TODO: anything else?
		};
	}
}
#endif