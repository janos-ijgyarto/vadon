#ifndef VADON_UTILITIES_TYPEINFO_ERASEDDATATYPE_HPP
#define VADON_UTILITIES_TYPEINFO_ERASEDDATATYPE_HPP
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Utilities
{
	enum class ErasedDataType
	{
		TRIVIAL,
		RESOURCE_ID,
		RESOURCE_HANDLE
	};

	struct ErasedDataTypeID
	{
		ErasedDataType type;
		std::underlying_type_t<TypeID> id; // NOTE: have to use underlying_type for simplicity in type erasure API
	};
}
#endif