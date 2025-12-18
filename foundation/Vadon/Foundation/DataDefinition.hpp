#ifndef VADON_FOUNDATION_DATA_DEFINITION_HPP
#define VADON_FOUNDATION_DATA_DEFINITION_HPP
namespace Vadon::DataDefinition
{
	// FIXME: use actual type list instead of enum to create more explicit link?
	enum class BaseType
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
		ARRAY,
		DICTIONARY
	};

	using UUIDDataType = unsigned char;

	struct UUID
	{
		static constexpr size_t c_uuid_width = 16;
		static constexpr UUIDDataType[c_uuid_width] c_invalid_uuid = { 0 };

		UUIDDataType data[c_uuid_width];
	};

	struct TypeInfo
	{
		UUID id;
		UUID base_id;
		uint32_t property_count;
	};

	struct Property
	{
		UUID id;
		UUID type_id;
	};
}
#endif