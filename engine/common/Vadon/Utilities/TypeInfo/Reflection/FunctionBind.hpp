#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_FUNCTIONBIND_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_FUNCTIONBIND_HPP
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

namespace Vadon::Utilities
{
	// TODO: non-member functions!
	// TODO2: virtual functions?
	// TODO3: variadics(???)

	template <typename T, typename Ret, typename... Args>
	using MemberFunction = Ret(T::*)(Args...);

	template <typename T, typename Ret, typename... Args>
	using ConstMemberFunction = Ret(T::*)(Args...) const;

	template <typename T, typename Ret, typename... Args>
	static constexpr std::vector<ErasedDataTypeID> make_argument_type_index_list(MemberFunction<T, Ret, Args...>)
	{
		return std::vector<ErasedDataTypeID> { get_erased_data_type_id<Args>()...};
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr std::vector<ErasedDataTypeID> make_argument_type_index_list(ConstMemberFunction<T, Ret, Args...>)
	{
		return std::vector<ErasedDataTypeID> { get_erased_data_type_id<Args>()...};
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr ErasedDataTypeID get_return_type_index(MemberFunction<T, Ret, Args...>)
	{
		if constexpr (std::is_void_v<Ret>)
		{
			return get_erased_data_type_id<NoReturnValue>();
		}
		else
		{
			return get_erased_data_type_id<Ret>();
		}
	}

	template <typename T, typename Ret, typename... Args>
	static constexpr ErasedDataTypeID get_return_type_index(ConstMemberFunction<T, Ret, Args...>)
	{
		if constexpr (std::is_void_v<Ret>)
		{
			return get_erased_data_type_id<NoReturnValue>();
		}
		else
		{
			return get_erased_data_type_id<Ret>();
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

	template <typename T, typename Ret, typename... Args>
	struct MemberFunctionInfo final
	{
		static constexpr size_t c_arg_count = sizeof...(Args);

		static Variant invoke(void* object, MemberFunction<T, Ret, Args...> function, VariantArgumentList args)
		{
			struct Invoker
			{
				using _ReturnType = std::conditional_t<std::is_void_v<Ret>, NoReturnValue, Ret>;

				Invoker(void* object, MemberFunction<T, Ret, Args...> function, Args&&... args)
				{
					T* cast_object = static_cast<T*>(object);
					if constexpr (std::is_void_v<Ret>)
					{
						(cast_object->*function)(std::forward<Args>(args)...);
					}
					else
					{
						return_value = (cast_object->*function)(std::forward<Args>(args)...);
					}
				}

				operator _ReturnType()
				{
					return std::move(return_value);
				}

				_ReturnType return_value;
			};

			size_t index = 0;
			if constexpr (std::is_void_v<Ret>)
			{
				Invoker{ object, function, std::forward<Args>(std::get<variant_type_mapping_t<Args>>(args[index++]))... };
				return NoReturnValue{};
			}
			else
			{
				const variant_type_mapping_t<Invoker::_ReturnType> mapped_return_value = Invoker{ object, function, std::forward<Args>(std::get<variant_type_mapping_t<Args>>(args[index++]))... }.return_value;
				return Variant(mapped_return_value);
			}
		}

		// NOTE: need separate overload for const functions
		static Variant invoke(void* object, ConstMemberFunction<T, Ret, Args...> function, VariantArgumentList args)
		{
			struct Invoker
			{
				using _ReturnType = std::conditional_t<std::is_void_v<Ret>, NoReturnValue, Ret>;

				Invoker(void* object, ConstMemberFunction<T, Ret, Args...> function, Args&&... args)
				{
					const T* cast_object = static_cast<T*>(object);
					if constexpr (std::is_void_v<Ret>)
					{
						(cast_object->*function)(std::forward<Args>(args)...);
					}
					else
					{
						return_value = (cast_object->*function)(std::forward<Args>(args)...);
					}
				}

				operator _ReturnType()
				{
					return std::move(return_value);
				}

				_ReturnType return_value;
			};

			size_t index = 0;
			if constexpr (std::is_void_v<Ret>)
			{
				Invoker{ object, function, std::forward<Args>(std::get<variant_type_mapping_t<Args>>(args[index++]))... };
				return NoReturnValue{};
			}
			else
			{
				const variant_type_mapping_t<Invoker::_ReturnType> mapped_return_value = Invoker{ object, function, std::forward<Args>(std::get<variant_type_mapping_t<Args>>(args[index++]))... }.return_value;
				return Variant(mapped_return_value);
			}
		}
	};

	template <typename T, typename Ret, typename... Args>
	constexpr auto get_member_function_info(Ret(T::*)(Args...))
	{
		return MemberFunctionInfo<T, Ret, Args...>{};
	}

	template <auto FPtr>
	ErasedMemberFunction erase_member_function()
	{
		using FunctionInfo = decltype(get_member_function_info(FPtr));
		return +[](void* obj, VariantArgumentList args)
			{
				return FunctionInfo::invoke(obj, FPtr, args);
			};
	}

	template <auto FPtr>
	MemberFunctionBind create_member_function_bind()
	{
		return MemberFunctionBind
		{
			.function = erase_member_function<FPtr>(),
			.argument_types = make_argument_type_index_list(FPtr),
			.return_type = get_return_type_index(FPtr),
		};
	}
}
#endif