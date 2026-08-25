#ifndef VADON_FOUNDATION_MODEL_ANIMATION_ANIMATION_HPP
#define VADON_FOUNDATION_MODEL_ANIMATION_ANIMATION_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct AnimationChannelSchema
		{
			static constexpr PropertySchema c_id_property { 
				"2bfb0c43-c71a-4394-b96c-a24801186d4a",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_key_times_property{
				"cfd81187-13db-4fee-9146-e28a73771a17",
				TypeInfo::get_base_type_uuid_string(BaseType::ARRAY)
			};

			static constexpr PropertySchema c_tag_property{
				"308276c8-4e4f-43a3-9b57-20cd4398dcf6",
				TypeInfo::get_base_type_uuid_string(BaseType::STRING)
			};

			enum Type
			{
				INT,
				UINT,
				FLOAT,
				VECTOR2,
				VECTOR2I,
				VECTOR3,
				VECTOR3I,
				VECTOR4,
				COLORRGBA,
				TYPE_COUNT,
				INVALID = TYPE_COUNT
			};

			static constexpr UUIDString c_base_type_uuid{ "d0a2c059-d27b-4866-a3ef-4d4bbb477f41" };

			static constexpr UUIDString c_channel_type_uuids[static_cast<uint32>(Type::TYPE_COUNT)] = { 
				UUIDString{"35472dfa-6161-4a39-8eba-c9df8460749e"},
				UUIDString{"ca797159-68d0-4a9f-96fc-5f5c2ce2223b"},
				UUIDString{"87534a15-e9e1-482e-bf75-cc49f29f187c"},
				UUIDString{"aca5c12a-4484-4d36-bd0d-6d1fa9d0025c"},
				UUIDString{"c63b992d-4daa-44b8-bac2-2fd60c1f2ade"},
				UUIDString{"d19264f6-31e6-4901-aa56-909469aa5589"},
				UUIDString{"62f60087-aab4-486b-b602-1fd187070a08"},
				UUIDString{"2ec3cb2f-a718-4802-b354-9cd856cc436d"},
				UUIDString{"79500c16-4620-4058-9884-f151d8278457"}
			};

			static constexpr UUIDString c_channel_data_property_uuids[static_cast<uint32>(Type::TYPE_COUNT)] = {
				UUIDString{"15f937ab-f2ad-4928-bbfa-8ff32a91c880"},
				UUIDString{"96df12e1-4e8a-463e-bea3-81ad48c0bea9"},
				UUIDString{"584344d6-475a-4f2a-8e68-c9d919ff76b2"},
				UUIDString{"9fd1ce7c-d221-4fc4-a7dc-d854cc684348"},
				UUIDString{"c35f1f2d-d9e1-486a-9dde-618d3c1470f2"},
				UUIDString{"1cc626a1-8ea2-4460-b35e-bab5cafdf53a"},
				UUIDString{"f4d12de8-c200-4eb8-adda-2c184bf52400"},
				UUIDString{"4824fe26-112c-4b4a-93ee-d4cdf5ffd243"},
				UUIDString{"6ff7807c-70a7-4972-8ff5-57eb812bee59"}
			};

			static constexpr UUIDString get_channel_type_uuid_string(Type channel_type)
			{
				return c_channel_type_uuids[static_cast<uint32>(channel_type)];
			}

			static constexpr UUIDString get_channel_data_property_uuid_string(Type channel_type)
			{
				return c_channel_data_property_uuids[static_cast<uint32>(channel_type)];
			}
		};

		struct AnimationSchema
		{
			static constexpr PropertySchema c_channels_property {
				"dcb03ddc-8fff-473d-ad92-6824e410b8a7",
				TypeInfo::get_base_type_uuid_string(BaseType::ARRAY)
			};

			static constexpr UUIDString c_type_uuid{ "45f3044d-0100-44af-9449-fee04590ce0d" };
		};
	}
}
#endif