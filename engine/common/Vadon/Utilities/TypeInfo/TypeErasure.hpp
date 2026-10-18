#ifndef VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <array>
namespace Vadon::Utilities
{
	// NOTE: these classes are distinct from TypeRegistryTrait. In most cases, types
	// will trivially use their TypeRegistryTrait for their type UUID, but some
	// types are not explicitly registered (e.g TypedObjectWrapper<T>) and instead
	// we want to map them onto a shared type. Some types (e.g containers) also need
	// to add the underlying type to the list.
	// 
	// FIXME: find a way to allow "shared implementations" and reduce duplicated code
	template<typename T>
	struct TypeErasureTrait
	{
		static constexpr ::Vadon::Foundation::UUID get_erased_type_uuid()
		{
			static_assert(false, "Type not supported!");
			return ::Vadon::Foundation::UUID{};
		}

		static constexpr size_t get_nested_type_count()
		{
			static_assert(false, "Type not supported!");
			return 0;
		}

		static constexpr void add_erased_type_uuid(::Vadon::Foundation::UUID* /*uuid_array*/)
		{
			static_assert(false, "Type not supported!");
		}

		static constexpr auto get_type_list()
		{
			std::array<::Vadon::Foundation::UUID, get_nested_type_count()> type_list;
			add_erased_type_uuid(type_list.data());
			return type_list;
		}
	};

	template<typename T>
	struct TrivialTypeErasureTrait
	{
		static constexpr ::Vadon::Foundation::UUID get_erased_type_uuid()
		{
			return TypeRegistryTrait<T>::get_type_uuid();
		}

		static constexpr size_t get_nested_type_count()
		{
			return 1;
		}

		static constexpr void add_erased_type_uuid(::Vadon::Foundation::UUID* uuid_array)
		{
			*uuid_array = get_erased_type_uuid();
		}

		static constexpr auto get_type_list()
		{
			std::array<::Vadon::Foundation::UUID, get_nested_type_count()> type_list;
			add_erased_type_uuid(type_list.data());
			return type_list;
		}
	};

	template<is_trivial_variant_type T>
	struct TypeErasureTrait<T> : public TrivialTypeErasureTrait<T> {};

	template<typename T>
	struct EnumTypeErasureTrait
	{
		static constexpr ::Vadon::Foundation::UUID get_erased_type_uuid()
		{
			return Vadon::Utilities::string_to_uuid(::Vadon::Foundation::EnumSchema::c_type_uuid);
		}

		static constexpr size_t get_nested_type_count()
		{
			// Enum "header" + the enum type itself
			return 2;
		}

		static constexpr void add_erased_type_uuid(::Vadon::Foundation::UUID* uuid_array)
		{
			*uuid_array = get_erased_type_uuid();

			++uuid_array;

			*uuid_array = TypeRegistryTrait<T>::get_type_uuid();
		}

		static constexpr auto get_type_list()
		{
			std::array<::Vadon::Foundation::UUID, get_nested_type_count()> type_list;
			add_erased_type_uuid(type_list.data());
			return type_list;
		}
	};

	struct EnumTypeTrait
	{
		VADONCOMMON_API static ::Vadon::Foundation::UUID to_uuid(const ::Vadon::Foundation::UUID& enum_type_uuid, size_t enum_value);
		VADONCOMMON_API static size_t to_enum_value(const ::Vadon::Foundation::UUID& enum_type_uuid, const ::Vadon::Foundation::UUID& enum_value_uuid);
	};

	template<typename T>
	struct EnumVariantTypeTrait
	{
		static Variant to_variant(const T& value)
		{
			const ::Vadon::Foundation::UUID enum_value_uuid = EnumTypeTrait::to_uuid(TypeRegistryTrait<T>::get_type_uuid(), static_cast<size_t>(value));
			return VariantTypeTrait<::Vadon::Foundation::UUID>::to_variant(enum_value_uuid);
		}

		static T from_variant(const Variant& variant)
		{
			const ::Vadon::Foundation::UUID enum_value_uuid = VariantTypeTrait<::Vadon::Foundation::UUID>::from_variant(variant);
			return static_cast<T>(EnumTypeTrait::to_enum_value(TypeRegistryTrait<T>::get_type_uuid(), enum_value_uuid));
		}
	};

	template<typename Container, typename T>
	struct ContainerTypeErasureTrait
	{
		static constexpr ::Vadon::Foundation::UUID get_erased_type_uuid()
		{
			return TypeErasureTrait<Container>::get_erased_type_uuid();
		}

		static constexpr size_t get_nested_type_count()
		{
			return 1 + TypeErasureTrait<T>::get_nested_type_count();
		}

		static constexpr void add_erased_type_uuid(::Vadon::Foundation::UUID* uuid_array)
		{
			*uuid_array = TypeErasureTrait<Container>::get_erased_type_uuid();

			++uuid_array;

			TypeErasureTrait<T>::add_erased_type_uuid(uuid_array);
		}

		static constexpr auto get_type_list()
		{
			std::array<::Vadon::Foundation::UUID, get_nested_type_count()> type_list;
			add_erased_type_uuid(type_list.data());
			return type_list;
		}
	};

	template<is_std_vector T>
	struct TypeErasureTrait<T> : public ContainerTypeErasureTrait<BoxedVariantArray, typename T::value_type> {};

	template<typename T>
	struct ObjectTypeErasureTrait : public TrivialTypeErasureTrait<T> {};

	template<>
	struct TypeErasureTrait<class ObjectWrapper> : public TrivialTypeErasureTrait<class ObjectWrapper> {};

	template<>
	struct TypeErasureTrait<class DataObject> : public TrivialTypeErasureTrait<class DataObject> {};

	template<typename T>
	struct TypedObjectWrapperTypeErasureTrait : public ContainerTypeErasureTrait<class ObjectWrapper, T> {};

	VADONCOMMON_API TypeID get_erased_data_type_id(TypeID type_id);
	VADONCOMMON_API Variant get_erased_type_default_value(TypeID type_id);
}

#define VADON_DEFINE_ENUM_TYPE_ERASURE(_enum_type) template<>\
struct Vadon::Utilities::TypeErasureTrait<_enum_type> : public Vadon::Utilities::EnumTypeErasureTrait<_enum_type> {}

#define VADON_DEFINE_ENUM_VARIANT_TYPE_TRAIT(_enum_type) template<>\
struct Vadon::Utilities::VariantTypeTrait<_enum_type> : public Vadon::Utilities::EnumVariantTypeTrait<_enum_type> {}

// Define for both the "bare" object and the typed wrapper version
#define VADON_DEFINE_OBJECT_TYPE_ERASURE(_type) template<>\
struct Vadon::Utilities::TypeErasureTrait<_type> : public Vadon::Utilities::ObjectTypeErasureTrait<_type> {}

#define VADON_DEFINE_TYPED_OBJECT_WRAPPER_TYPE_ERASURE(_type) template<>\
struct Vadon::Utilities::TypeErasureTrait<Vadon::Utilities::TypedObjectWrapper<_type>> : public TypedObjectWrapperTypeErasureTrait<_type> {}

#define VADON_DEFINE_OBJECT_AND_WRAPPER_TYPE_ERASURE(_type) VADON_DEFINE_OBJECT_TYPE_ERASURE(_type);\
VADON_DEFINE_TYPED_OBJECT_WRAPPER_TYPE_ERASURE(_type)
#endif