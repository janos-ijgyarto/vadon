#ifndef VADON_UTILITIES_TYPEINFO_DATA_HPP
#define VADON_UTILITIES_TYPEINFO_DATA_HPP
#include <Vadon/Utilities/TypeInfo/Registry/FunctionBind.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>
namespace Vadon::Utilities
{
	class TypeDataInterface
	{
	public:
		template <auto Ptr>
		bool bind_method(std::string_view name)
		{
			return internal_bind_method(name, std::move(create_member_function_bind<Ptr>()));
		}

		virtual bool add_property(std::string_view name, MemberVariableBindBase property_bind) = 0;
	protected:
		virtual bool internal_bind_method(std::string_view name, MemberFunctionBind method_bind) = 0;
	};
}
#define VADON_METHOD_BIND(_method) decltype(&_method), &_method
#endif