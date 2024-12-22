#ifndef VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#include <string>
#include <vector>
namespace Vadon::Utilities
{
	enum class TypeID : uint32_t { INVALID = 0 };

	struct TypeInfo
	{
		TypeID id = TypeID::INVALID;
		TypeID base_id = TypeID::INVALID;
		std::string name;
		size_t size;
		size_t alignment;

		bool is_valid() const { return (id != TypeID::INVALID); }
	};

	using TypeInfoList = std::vector<TypeInfo>;
}
#endif