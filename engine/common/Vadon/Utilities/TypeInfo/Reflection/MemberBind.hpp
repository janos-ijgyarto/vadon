#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_MEMBERBIND_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_MEMBERBIND_HPP
#include <Vadon/Utilities/TypeInfo/Reflection/FunctionBind.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBindBase.hpp>
namespace Vadon::Utilities
{
	template <typename T, typename TMember>
	struct MemberPointerInfo
	{
		using _ObjectType = T;
		using _MemberType = TMember;

		static void* get_member_address(void* object, TMember T::* member_ptr)
		{
			T* cast_object = static_cast<T*>(object);
			return &(cast_object->*member_ptr);
		}

		static Variant invoke_getter(void* object, TMember T::* member_ptr)
		{
			T* cast_object = static_cast<T*>(object);
			return VariantTypeTrait<_MemberType>::to_variant(cast_object->*member_ptr);
		}

		template <typename T, typename TMember>
		static void invoke_setter(void* object, TMember T::* member_ptr, const Variant& value)
		{
			T* cast_object = static_cast<T*>(object);
			cast_object->*member_ptr = VariantTypeTrait<_MemberType>::from_variant(value);
		}
	};

	template <typename T, typename TMember>
	constexpr auto get_member_pointer_info(TMember T::*)
	{
		return MemberPointerInfo<T, TMember>{};
	}

	template<auto MemberPtr>
	ErasedMemberAddressGetter erase_member_address_getter()
	{
		using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
		return +[](void* obj)
			{
				return MemberInfo::get_member_address(obj, MemberPtr);
			};
	}

	template<auto MemberPtr>
	ErasedMemberGetter erase_member_getter()
	{
		using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
		return +[](void* obj)
			{
				return MemberInfo::invoke_getter(obj, MemberPtr);
			};
	}

	template<auto MemberPtr>
	ErasedMemberSetter erase_member_setter()
	{
		using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
		return +[](void* obj, const Variant& value)
			{
				MemberInfo::invoke_setter(obj, MemberPtr, value);
			};
	}

	template<auto MemberPtr>
	struct MemberVariableBind : public MemberVariableBindBase
	{
		constexpr MemberVariableBind()
		{
			using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
			constexpr auto type_list_array = TypeErasureTrait<MemberInfo::_MemberType>::get_type_list();
			type_list = std::vector<::Vadon::Foundation::UUID>(type_list_array.begin(), type_list_array.end());
		}

		MemberVariableBind& bind_member_address_getter()
		{
			address_getter = erase_member_address_getter<MemberPtr>();
			return *this;
		}

		MemberVariableBind& bind_member_getter()
		{
			member_getter = erase_member_getter<MemberPtr>();
			return *this;
		}

		MemberVariableBind& bind_member_setter()
		{
			// TODO: assert if function already set!
			member_setter = erase_member_setter<MemberPtr>();
			return *this;
		}

		template<auto FunctionPtr>
		MemberVariableBind& bind_getter_function()
		{
			// TODO: assert if member getter already set!
			// TODO2: assert if getter type doesn't match member type!
			getter_function = erase_member_function<FunctionPtr>();
			return *this;
		}

		template<auto FunctionPtr>
		MemberVariableBind& bind_setter_function()
		{
			// TODO: assert if member setter already set!
			// TODO2: assert if setter type doesn't match member type!
			setter_function = erase_member_function<FunctionPtr>();
			return *this;
		}
	};

	template<typename T, auto MemberPtr>
	constexpr MemberVariableBind<MemberPtr> create_member_variable_bind()
	{
		using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
		static_assert(std::is_same_v<typename MemberInfo::_ObjectType, T>, "A type may only register its own members!");
		return MemberVariableBind<MemberPtr>();
	}
}
#endif