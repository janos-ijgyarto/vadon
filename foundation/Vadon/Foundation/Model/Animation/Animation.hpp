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

			static constexpr PropertySchema c_sample_method_property{
				"8cc157c9-1315-4196-9f2a-90417f4b0f7f",
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
				UUID,
				TYPE_COUNT,
				INVALID_TYPE = TYPE_COUNT
			};

			enum SampleMethod
			{
				LINEAR,
				NEAREST_NEIGHBOR,
				LAST_KEY, // Similar to NN but it is set to whichever key was last passed
				METHOD_COUNT,
				INVALID_METHOD = METHOD_COUNT
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
				UUIDString{"79500c16-4620-4058-9884-f151d8278457"},
				UUIDString{"b27e6e19-51aa-4608-870b-1904a414a742"}
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
				UUIDString{"6ff7807c-70a7-4972-8ff5-57eb812bee59"},
				UUIDString{"904c77f2-85d5-4bca-94bd-53f02f5f4362"}
			};

			static constexpr UUIDString c_sample_method_uuids[static_cast<uint32>(SampleMethod::METHOD_COUNT)] = {
				UUIDString{"76d0d4b4-a061-4296-918d-814646de569c"},
				UUIDString{"a26f8ed8-2df1-4e92-a541-d257e40dd11e"},
				UUIDString{"89f7add7-a6ed-4099-9669-917a5b412fa2"},
			};

			static constexpr UUIDString get_channel_type_uuid_string(Type channel_type)
			{
				return c_channel_type_uuids[static_cast<uint32>(channel_type)];
			}

			static constexpr UUIDString get_channel_data_property_uuid_string(Type channel_type)
			{
				return c_channel_data_property_uuids[static_cast<uint32>(channel_type)];
			}

			static constexpr UUIDString get_sample_method_uuid_string(SampleMethod sample_method)
			{
				return c_sample_method_uuids[static_cast<uint32>(sample_method)];
			}

			static constexpr UUIDString c_sample_method_enum_uuid{ "d1ff2f58-1a83-43fe-867f-25d27a1481cb" };
		};

		struct AnimationSchema
		{
			static constexpr PropertySchema c_channels_property {
				"dcb03ddc-8fff-473d-ad92-6824e410b8a7",
				TypeInfo::get_base_type_uuid_string(BaseType::ARRAY)
			};

			static constexpr PropertySchema c_time_scale_property {
				"79a926cb-0aad-4514-86e7-7f2601e9eb46",
				TypeInfo::get_base_type_uuid_string(BaseType::FLOAT)
			};

			static constexpr PropertySchema c_looping_property{
				"d07dc003-d4df-4de3-95e8-55b939ec3419",
				TypeInfo::get_base_type_uuid_string(BaseType::BOOL)
			};			

			static constexpr UUIDString c_type_uuid{ "45f3044d-0100-44af-9449-fee04590ce0d" };
		};

		struct AnimationLibrarySchema
		{
			static constexpr PropertySchema c_animations_property{
				"4d47c31e-73e5-43f9-ba39-eb52ce4d61ad",
				TypeInfo::get_base_type_uuid_string(BaseType::ARRAY)
			};

			static constexpr UUIDString c_type_uuid{ "32c9cdf8-7551-42f7-b675-757109c564fe" };
		};

		struct EntityAnimationEntrySchema
		{
			static constexpr PropertySchema c_animation_property{
				"579b407a-f739-4ab9-bb77-96b5008e6741",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_channel_property{
				"171a93f3-e286-457f-9d39-13a9881a0b6a",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_entity_property{
				"483aff97-c885-4437-9a76-3c633e61f267",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_component_property{
				"26db0129-b265-4a24-9dec-26c5e86336f9",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr PropertySchema c_property_id_property{
				"3d710103-e549-49ea-a212-6de4f90f0da3",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr UUIDString c_type_uuid{ "989bf8a7-3999-4258-8398-57c31480836e" };
		};

		struct EntityAnimationSchema
		{
			static constexpr PropertySchema c_entries_property{
				"b1eee5dc-e2c6-4454-aebc-a215fd29d1db",
				TypeInfo::get_base_type_uuid_string(BaseType::ARRAY)
			};

			static constexpr UUIDString c_type_uuid{ "138ed74b-ca11-45bd-9f24-4209b426a136" };
		};

		// NOTE: animation component maps channels from an animation
		// to properties of components in entities (either same entity
		// or even another entity in the same scene)
		struct AnimationComponentSchema
		{
			static constexpr PropertySchema c_animation_property {
				"cff15424-c6da-41ca-8580-8a9e239442d1",
				TypeInfo::get_base_type_uuid_string(BaseType::UUID)
			};

			static constexpr UUIDString c_type_uuid{ "bef5235a-d25e-4317-8eff-c425da27e8ba" };
		};
	}
}
#endif