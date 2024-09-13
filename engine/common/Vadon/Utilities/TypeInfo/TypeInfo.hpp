#ifndef VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#include <string>
#include <vector>
namespace Vadon::Utilities
{
	using TypeID = uint32_t;
	constexpr TypeID c_invalid_type_id = TypeID(0);

	struct TypeInfo
	{
		TypeID id;
		TypeID base_id = c_invalid_type_id;
		std::string name;
		size_t size;
		size_t alignment;

		bool is_valid() const { return (id != c_invalid_type_id); }
	};

	using TypeInfoList = std::vector<TypeInfo>;
}
#endif