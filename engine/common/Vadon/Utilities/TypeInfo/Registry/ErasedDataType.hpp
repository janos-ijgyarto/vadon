#ifndef VADON_UTILITIES_TYPEINFO_ERASEDDATATYPE_HPP
#define VADON_UTILITIES_TYPEINFO_ERASEDDATATYPE_HPP
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Utilities
{
	enum class ErasedDataType
	{
		TRIVIAL,
		RESOURCE_HANDLE
	};

	struct ErasedDataTypeID
	{
		ErasedDataType type;
		Vadon::Utilities::TypeID id;
	};
}
#endif