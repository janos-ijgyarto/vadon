#ifndef VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEINFO_HPP
#include <string>
#include <vector>
namespace Vadon::Utilities
{
	struct TypeInfo
	{
		uint32_t id;
		std::string name;

		bool is_valid() const { return (id != 0); }
	};

	using TypeInfoList = std::vector<TypeInfo>;
}
#endif