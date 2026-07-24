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

	template<typename T>
	struct TypedObjectWrapperTypeErasureTrait : public ContainerTypeErasureTrait<class ObjectWrapper, T> {};

	VADONCOMMON_API TypeID get_erased_data_type_id(TypeID type_id);
	VADONCOMMON_API Variant get_erased_type_default_value(TypeID type_id);
}

// Define for both the "bare" object and the typed wrapper version
#define VADON_DEFINE_OBJECT_TYPE_ERASURE(_type) template<>\
struct Vadon::Utilities::TypeErasureTrait<_type> : public Vadon::Utilities::ObjectTypeErasureTrait<_type> {};\
template<>\
struct Vadon::Utilities::TypeErasureTrait<Vadon::Utilities::TypedObjectWrapper<_type>> : public TypedObjectWrapperTypeErasureTrait<_type> {};
#endif