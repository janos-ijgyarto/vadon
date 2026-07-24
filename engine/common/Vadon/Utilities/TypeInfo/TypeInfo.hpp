#ifndef VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#include <Vadon/Utilities/Data/Variant.hpp>

#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>

namespace Vadon::Utilities
{
	// NOTE: this is a runtime type ID, assigned to each registered type at runtime
	// The persistent unique ID is a UUID
	enum class TypeID : uint32_t { INVALID = 0 };

	using TypeUUID = ::Vadon::Foundation::UUID;

	// TODO: could allow client code to use overloads to add other metadata to the types?
	template<typename T>
	struct TypeRegistryTrait
	{
		static constexpr TypeUUID get_type_uuid()
		{
			static_assert(false, "Did not register UUID for type!");
			return TypeUUID{};
		}
	};

	template<typename T>
	struct BaseDataTypeTrait : public std::false_type {};
}

#define VADON_REGISTER_TYPE_UUID(_type, _uuid_str) template<> \
struct ::Vadon::Utilities::TypeRegistryTrait<_type>\
{\
	static constexpr ::Vadon::Foundation::UUID get_type_uuid()\
	{\
		return UUIDLiteral(_uuid_str).result;\
	}\
}\

#define VADON_GET_UUID_BASE64_STRING(_uuid) ::Vadon::Utilities::uuid_to_base64_string(_uuid).c_str()

#define VADON_GET_TYPE_UUID(_type) ::Vadon::Utilities::TypeRegistryTrait<_type>::get_type_uuid()
#define VADON_GET_TYPE_UUID_BASE64_STRING(_type) VADON_GET_UUID_BASE64_STRING(VADON_GET_TYPE_UUID(_type))

#define VADON_MEMBER_UUID_BASE(_name) c_##_name##_member_id
#define VADON_DECLARE_MEMBER_UUID(_name, _uuid_str) static constexpr auto VADON_MEMBER_UUID_BASE(_name) = Vadon::Utilities::UUIDLiteral(_uuid_str).result
#define VADON_GET_MEMBER_UUID(_type, _name) _type##::VADON_MEMBER_UUID_BASE(_name)
#define VADON_GET_MEMBER_UUID_BASE64_STRING(_type, _name) VADON_GET_UUID_BASE64_STRING(VADON_GET_MEMBER_UUID(_type, _name))

#define VADON_DECLARE_BASE_DATA_TYPE(_type) template<> struct BaseDataTypeTrait<_type> : public ::std::true_type {}

namespace Vadon::Utilities
{
	VADON_REGISTER_TYPE_UUID(int, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::INT32).string);
	VADON_DECLARE_BASE_DATA_TYPE(int);

	VADON_REGISTER_TYPE_UUID(uint32_t, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::UINT32).string);
	VADON_DECLARE_BASE_DATA_TYPE(uint32_t);

	VADON_REGISTER_TYPE_UUID(float, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::FLOAT).string);
	VADON_DECLARE_BASE_DATA_TYPE(float);

	VADON_REGISTER_TYPE_UUID(bool, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::BOOL).string);
	VADON_DECLARE_BASE_DATA_TYPE(bool);

	VADON_REGISTER_TYPE_UUID(std::string, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::STRING).string);
	VADON_DECLARE_BASE_DATA_TYPE(std::string);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector2, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR2).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector2);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector2i, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR2I).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector2i);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector3, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR3).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector3);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector3i, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR3I).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector3i);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector4, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR4).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector4);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::ColorRGBA, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::COLORRGBA).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::ColorRGBA);

	VADON_REGISTER_TYPE_UUID(Vadon::Foundation::UUID, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::UUID).string);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Foundation::UUID);

	VADON_REGISTER_TYPE_UUID(NoReturnValue, "82a04f80-b040-4154-94a6-8a0212f5cff8");
	VADON_DECLARE_BASE_DATA_TYPE(NoReturnValue);

	VADON_REGISTER_TYPE_UUID(BoxedVariantArray, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::ARRAY).string);
	VADON_DECLARE_BASE_DATA_TYPE(BoxedVariantArray);

	VADON_REGISTER_TYPE_UUID(BoxedVariantDictionary, ::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(::Vadon::Foundation::BaseType::DICTIONARY).string);
	VADON_DECLARE_BASE_DATA_TYPE(BoxedVariantDictionary);

	constexpr ::Vadon::Foundation::UUID get_base_type_uuid(::Vadon::Foundation::BaseType base_type)
	{
		return UUIDLiteral(::Vadon::Foundation::TypeInfo::get_base_type_uuid_string(base_type).string).result;
	}

	constexpr ::Vadon::Foundation::BaseType base_type_from_uuid(const ::Vadon::Foundation::UUID& type_uuid)
	{
		for (uint32_t base_type_index = 0; base_type_index < static_cast<uint32_t>(::Vadon::Foundation::BaseType::TYPE_COUNT); ++base_type_index)
		{
			const ::Vadon::Foundation::BaseType current_base_type = static_cast<::Vadon::Foundation::BaseType>(base_type_index);
			if (type_uuid == get_base_type_uuid(current_base_type))
			{
				return current_base_type;
			}
		}

		return ::Vadon::Foundation::BaseType::INVALID;
	}

	constexpr bool is_base_type(const ::Vadon::Foundation::UUID& type_uuid)
	{
		return base_type_from_uuid(type_uuid) != ::Vadon::Foundation::BaseType::INVALID;
	}
}

#endif