#ifndef VADON_UTILITIES_TYPEINFO_MEMBERBINDBASE_HPP
#define VADON_UTILITIES_TYPEINFO_MEMBERBINDBASE_HPP
#include <Vadon/Utilities/TypeInfo/Registry/ErasedDataType.hpp>
namespace Vadon::Utilities
{
	using ErasedMemberGetter = Variant(*)(void*);
	using ErasedMemberSetter = void(*)(void*, const Variant&);

	struct MemberVariableBindBase
	{
		ErasedDataTypeID data_type;

		ErasedMemberGetter member_getter = nullptr;
		ErasedMemberFunction getter_function = nullptr;

		ErasedMemberSetter member_setter = nullptr;
		ErasedMemberFunction setter_function = nullptr;

		bool has_getter() const { return member_getter || getter_function; }
		bool has_setter() const { return member_setter || setter_function; }
	};
}
#endif