#ifndef VADON_CORE_CORE_HPP
#define VADON_CORE_CORE_HPP
#include <Vadon/Core.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;

	using EngineCoreImpl = std::unique_ptr<EngineCoreInterface>;

	VADONCORE_API EngineCoreImpl create_engine_core();
}
#endif