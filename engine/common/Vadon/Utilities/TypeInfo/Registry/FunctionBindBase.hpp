#ifndef VADON_UTILITIES_TYPEINFO_FUNCTIONBINDBASE_HPP
#define VADON_UTILITIES_TYPEINFO_FUNCTIONBINDBASE_HPP
#include <Vadon/Utilities/TypeInfo/Registry/ErasedDataType.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
#include <span>
namespace Vadon::Utilities
{
	using VariantArgumentList = std::span<Variant>;

	using ErasedMemberFunction = Variant(*)(void*, VariantArgumentList args);

	struct MemberFunctionBind
	{
		ErasedMemberFunction function;
		std::vector<ErasedDataTypeID> argument_types;
		ErasedDataTypeID return_type;
	};
}
#endif