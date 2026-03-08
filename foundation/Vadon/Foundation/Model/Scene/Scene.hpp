#ifndef VADON_FOUNDATION_MODEL_SCENE_SCENE_HPP
#define VADON_FOUNDATION_MODEL_SCENE_SCENE_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct SceneSchema
		{
			static constexpr UUIDString c_type_uuid{ "c432094b-d5f2-4e80-9c95-2cd669a8ed1e" };

			static constexpr PropertySchema c_entities_property{
				UUIDString{"b91e01e7-49b9-441a-9092-bbda48fafce1"},
				get_base_type_uuid_string(BaseType::ARRAY)
			};
		};

		struct SceneEntitySchema
		{
			static constexpr PropertySchema c_components_property{
				UUIDString{ "f5c4290e-dbcd-411d-93b8-ddf662be3e8a" },
				get_base_type_uuid_string(BaseType::ARRAY)
			};
			static constexpr PropertySchema c_name_property{
				UUIDString{ "e9ea6842-c532-4417-bfd0-77dab9fd45fb" },
				get_base_type_uuid_string(BaseType::STRING)
			};
			static constexpr PropertySchema c_parent_property{
				UUIDString{ "2704e0c4-40e8-4f29-baa8-a6acf4369530" },
				get_base_type_uuid_string(BaseType::INT32)
			};
			static constexpr PropertySchema c_scene_property{
				UUIDString{ "7ae2b1c0-da76-46df-9f5f-722c9a4f0742" },
				get_base_type_uuid_string(BaseType::UUID)
			};
		};

		struct SceneEntityComponentSchema
		{
			static constexpr PropertySchema c_properties_property {
				UUIDString{ "386de6f0-4a26-453d-a901-916ab9b5b46c" },
				get_base_type_uuid_string(BaseType::DICTIONARY)
			};
			static constexpr PropertySchema c_type_property {
				UUIDString{ "ad2187ce-346b-484c-a0b0-3e057a949c15" },
				get_base_type_uuid_string(BaseType::UUID)
			};
		};
	}
}
#endif