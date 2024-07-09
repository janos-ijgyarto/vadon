#ifndef VADON_UTILITIES_TYPEINFO_MEMBERBIND_HPP
#define VADON_UTILITIES_TYPEINFO_MEMBERBIND_HPP
#include <Vadon/Utilities/TypeInfo/Registry/FunctionBind.hpp>
namespace Vadon::Utilities
{
	template <typename T, typename TMember>
	struct MemberPointerInfo
	{
		using _ObjectType = T;
		using _MemberType = TMember;

		static Variant invoke_getter(void* object, TMember T::* member_ptr)
		{
			T* cast_object = static_cast<T*>(object);
			return Variant(cast_object->*member_ptr);
		}

		template <typename T, typename TMember>
		static void invoke_setter(void* object, TMember T::* member_ptr, const Variant& value)
		{
			T* cast_object = static_cast<T*>(object);
			cast_object->*member_ptr = std::get<TMember>(value);
		}
	};

	template <typename T, typename TMember>
	constexpr auto get_member_pointer_info(TMember T::*)
	{
		return MemberPointerInfo<T, TMember>{};
	}

	using ErasedMemberGetter = Variant(*)(void*);
	using ErasedMemberSetter = void(*)(void*, const Variant&);

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

	struct MemberVariableBindBase
	{
		size_t type;

		ErasedMemberGetter member_getter = nullptr;
		ErasedMemberFunction getter_function = nullptr;

		ErasedMemberSetter member_setter = nullptr;
		ErasedMemberFunction setter_function = nullptr;

		bool has_getter() const { return member_getter || getter_function; }
		bool has_setter() const { return member_setter || setter_function; }
	};

	template<auto MemberPtr>
	struct MemberVariableBind : public MemberVariableBindBase
	{
		constexpr MemberVariableBind()
		{
			using MemberInfo = decltype(get_member_pointer_info(MemberPtr));
			type = type_list_index_v<MemberInfo::_MemberType, Variant>;
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
}
#endif