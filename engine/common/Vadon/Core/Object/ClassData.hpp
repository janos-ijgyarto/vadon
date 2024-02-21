#ifndef VADON_CORE_OBJECT_CLASSDATA_HPP
#define VADON_CORE_OBJECT_CLASSDATA_HPP
#include <Vadon/Core/Object/FunctionBind.hpp>

namespace Vadon::Core
{
	struct ObjectProperty
	{
		std::string name;
		size_t type_index;
		std::string getter;
		std::string setter;
	};

	class ObjectClassData
	{
	public:
		template <typename TFunc, TFunc Ptr>
		bool bind_method(std::string_view name)
		{
			return internal_bind_method(name, std::move(create_member_function_data<TFunc, Ptr>()));
		}

		virtual bool add_property(ObjectProperty property) = 0;
	protected:
		virtual bool internal_bind_method(std::string_view name, MemberFunctionData method_data) = 0;
	};
}

#define VADON_METHOD_BIND(_method) decltype(&_method), &_method

#define VADON_OBJECT_PROPERTY(_name, _member, _getter, _setter) Vadon::Core::ObjectProperty{ .name = _name, .type_index = Vadon::Core::type_list_index_v<decltype(_member), Vadon::Core::Variant>, .getter = _getter, .setter = _setter }
#define VADON_ADD_OBJECT_PROPERTY(_name, _member, _getter, _setter) std::move(VADON_OBJECT_PROPERTY(_name, _member, _getter, _setter))
#endif