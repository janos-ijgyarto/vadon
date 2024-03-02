#ifndef VADON_CORE_OBJECT_CLASSDATA_HPP
#define VADON_CORE_OBJECT_CLASSDATA_HPP
#include <Vadon/Core/Object/FunctionBind.hpp>
#include <Vadon/Core/Object/Property.hpp>

namespace Vadon::Core
{
	class ObjectClassData
	{
	public:
		template <typename TFunc, TFunc Ptr>
		bool bind_method(std::string_view name)
		{
			return internal_bind_method(name, std::move(create_member_function_data<TFunc, Ptr>()));
		}

		virtual bool add_property(ObjectPropertyInfo property) = 0;
	protected:
		virtual bool internal_bind_method(std::string_view name, MemberFunctionData method_data) = 0;
	};
}

#define VADON_METHOD_BIND(_method) decltype(&_method), &_method
#endif