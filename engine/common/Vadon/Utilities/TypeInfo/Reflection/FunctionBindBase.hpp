#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_FUNCTIONBINDBASE_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_FUNCTIONBINDBASE_HPP
#include <Vadon/Utilities/Data/Variant.hpp>
#include <span>
namespace Vadon::Utilities
{
	using MemberFunctionUUID = ::Vadon::Foundation::UUID;

	using VariantArgumentList = std::span<Variant>;

	using ErasedMemberFunction = Variant(*)(void*, VariantArgumentList args);

	struct MemberFunctionBind
	{
		ErasedMemberFunction function;
		std::vector<TypeID> argument_types;
		TypeID return_type;
	};
}
#endif