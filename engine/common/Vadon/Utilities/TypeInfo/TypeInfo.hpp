#ifndef VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#include <Vadon/Math/Color.hpp>
#include <Vadon/Math/Vector.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>

namespace Vadon::Utilities
{
	// NOTE: this is a runtime type ID, assigned to each registered type at runtime
	// The persistent unique ID is a UUID
	enum class TypeID : uint32_t { INVALID = 0 };

	using TypeUUID = ::Vadon::Foundation::UUID;

	// TODO: could allow client code to use overloads to add other metadata to the types?
	template<typename T, typename Base = T>
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

#define VADON_GET_TYPE_UUID(_type) ::Vadon::Utilities::TypeRegistryTrait<_type>::get_type_uuid()

#define VADON_MEMBER_UUID_BASE(_name) c_##_name##_member_id
#define VADON_DECLARE_MEMBER_UUID(_name, _uuid_str) static constexpr auto VADON_MEMBER_UUID_BASE(_name) = Vadon::Utilities::UUIDLiteral(_uuid_str).result
#define VADON_GET_MEMBER_UUID(_type, _name) _type##::VADON_MEMBER_UUID_BASE(_name)

#define VADON_DECLARE_BASE_DATA_TYPE(_type) template<> struct BaseDataTypeTrait<_type> : public ::std::true_type {}

namespace Vadon::Utilities
{
	VADON_REGISTER_TYPE_UUID(int, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::INT32)]);
	VADON_DECLARE_BASE_DATA_TYPE(int);

	VADON_REGISTER_TYPE_UUID(uint32_t, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::UINT32)]);
	VADON_DECLARE_BASE_DATA_TYPE(uint32_t);

	VADON_REGISTER_TYPE_UUID(float, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::FLOAT)]);
	VADON_DECLARE_BASE_DATA_TYPE(float);

	VADON_REGISTER_TYPE_UUID(bool, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::BOOL)]);
	VADON_DECLARE_BASE_DATA_TYPE(bool);

	VADON_REGISTER_TYPE_UUID(std::string, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::STRING)]);
	VADON_DECLARE_BASE_DATA_TYPE(std::string);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector2, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::VECTOR2)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector2);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector2i, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::VECTOR2I)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector2i);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector3, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::VECTOR3)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector3);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector3i, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::VECTOR3I)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector3i);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::Vector4, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::VECTOR4)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::Vector4);

	VADON_REGISTER_TYPE_UUID(Vadon::Math::ColorRGBA, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::COLORRGBA)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Math::ColorRGBA);

	VADON_REGISTER_TYPE_UUID(Vadon::Foundation::UUID, ::Vadon::Foundation::c_base_type_uuids[static_cast<uint32_t>(::Vadon::Foundation::BaseType::UUID)]);
	VADON_DECLARE_BASE_DATA_TYPE(Vadon::Foundation::UUID);
}

#endif