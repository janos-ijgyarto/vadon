#ifndef VADON_CORE_OBJECT_CLASSINFO_HPP
#define VADON_CORE_OBJECT_CLASSINFO_HPP
#include <vector>
#include <string>
namespace Vadon::Core
{
	struct ObjectClassInfo
	{
		std::string id;
		std::string base_id;
		std::string name;

		bool is_valid() const { return (id.empty() == false); }
	};

	using ObjectClassInfoList = std::vector<ObjectClassInfo>;
}
#endif