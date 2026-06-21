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
			static constexpr UUIDString c_type_uuid{ "6fb6cc0a-2a14-47e2-88bd-7b3322b51a58" };

			static constexpr PropertySchema c_id_property{
				UUIDString{ "c3560d83-ea76-48e3-8d8a-e2298bb65bcd" },
				get_base_type_uuid_string(BaseType::UUID)
			};
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
				get_base_type_uuid_string(BaseType::UUID)
			};
			static constexpr PropertySchema c_scene_property{
				UUIDString{ "7ae2b1c0-da76-46df-9f5f-722c9a4f0742" },
				get_base_type_uuid_string(BaseType::UUID)
			};
		};

		struct SceneEntityInfo
		{
			UUID id;
			UUID parent;
			UUID sub_scene;
		};
	}
}
#endif