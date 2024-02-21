#ifndef VADON_CORE_OBJECT_FUNCTIONBIND_HPP
#define VADON_CORE_OBJECT_FUNCTIONBIND_HPP
#include <Vadon/Core/Object/Variant.hpp>
#include <Vadon/Utilities/Type/TypeList.hpp>

#include <span>

namespace Vadon::Core
{
	class Object;

	// TODO: non-member functions!
	// TODO2: virtual functions?
	// TODO3: variadics(???)

	template <typename T, typename Ret, typename... Args>
	using MemberFunction = Ret(T::*)(Args...);

	template <typename T, typename Ret, typename... Args>
	using ConstMemberFunction = Ret(T::*)(Args...) const;

	using VariantArgumentList = std::span<Variant>;

	using ErasedMemberFunction = Variant(*)(Object*, VariantArgumentList args);

	// FIXME: for some reason I cannot use the one that's in the Utilities namespace?
	template <typename T, typename... Alts>
	static constexpr size_t type_list_index_v = Utilities::TypeList<Alts...>::template get_type_id<T>();

	template <typename T, typename... Alts>
	static constexpr size_t type_list_index_v<T, std::variant<Alts...>> = Utilities::TypeList<Alts...>::template get_type_id<T>();

	template <typename T, typename Ret, typename... Args>
	static constexpr std::vector<size_t> make_argument_type_index_list(MemberFunction<T, Ret, Args...>)
	{
		return std::vector<size_t> {type_list_index_v<variant_type_mapping_t<Args>, Variant>...};
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr std::vector<size_t> make_argument_type_index_list(ConstMemberFunction<T, Ret, Args...>)
	{
		return std::vector<size_t> {type_list_index_v<variant_type_mapping_t<Args>, Variant>...};
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr size_t get_return_type_index(MemberFunction<T, Ret, Args...>)
	{
		if constexpr (std::is_void_v<Ret>)
		{
			return type_list_index_v<NoReturnValue, Variant>;
		}
		else
		{
			return type_list_index_v<variant_type_mapping_t<Ret>, Variant>;
		}		
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr size_t get_return_type_index(ConstMemberFunction<T, Ret, Args...>)
	{
		if constexpr (std::is_void_v<Ret>)
		{
			return type_list_index_v<NoReturnValue, Variant>;
		}
		else
		{
			return type_list_index_v<variant_type_mapping_t<Ret>, Variant>;
		}
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr size_t get_argument_count(MemberFunction<T, Ret, Args...>)
	{
		return sizeof...(Args);
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr size_t get_argument_count(ConstMemberFunction<T, Ret, Args...>)
	{
		return sizeof...(Args);
	}

	template <typename T, typename Ret, typename... Args, size_t... Indices>
	Variant invoke_member_function_impl(Object* object, MemberFunction<T, Ret, Args...> function, VariantArgumentList args, std::index_sequence<Indices...>)
	{
		T* cast_object = static_cast<T*>(object);

		if constexpr (std::is_void_v<Ret>)
		{
			(cast_object->*function)(std::get<variant_type_mapping_t<Args>>(args[Indices])...);
			return NoReturnValue();
		}
		else
		{
			return Variant((cast_object->*function)(std::get<variant_type_mapping_t<Args>>(args[Indices])...));
		}
	}

	// NOTE: need separate overload for const functions
	template <typename T, typename Ret, typename... Args, size_t... Indices>
	Variant invoke_member_function_impl(Object* object, ConstMemberFunction<T, Ret, Args...> function, VariantArgumentList args, std::index_sequence<Indices...>)
	{
		const T* cast_object = static_cast<T*>(object);

		if constexpr (std::is_void_v<Ret>)
		{
			(cast_object->*function)(std::get<variant_type_mapping_t<Args>>(args[Indices])...);
			return NoReturnValue;
		}
		else
		{
			return Variant((cast_object->*function)(std::get<variant_type_mapping_t<Args>>(args[Indices])...));
		}
	}

	template <typename TFunc, TFunc FPtr>
	ErasedMemberFunction erase_member_function()
	{
		return +[](Object* obj, VariantArgumentList args)
			{
				return invoke_member_function_impl(obj, FPtr, args, std::make_index_sequence<get_argument_count(FPtr)>());
			};
	}

	struct MemberFunctionData
	{
		ErasedMemberFunction function;
		std::vector<size_t> argument_types;
		size_t return_type;
	};

	template <typename TFunc, TFunc FPtr>
	MemberFunctionData create_member_function_data()
	{
		return MemberFunctionData
		{
			.function = erase_member_function<TFunc, FPtr>(),
			.argument_types = make_argument_type_index_list(FPtr),
			.return_type = get_return_type_index(FPtr),
		};
	}
}
#endif