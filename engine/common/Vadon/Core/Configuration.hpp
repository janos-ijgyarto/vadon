#ifndef VADON_CORE_CONFIGURATION_HPP
#define VADON_CORE_CONFIGURATION_HPP
#include <Vadon/Core/Task/Configuration.hpp>
namespace Vadon::Core
{
	struct CoreConfiguration
	{
		std::string program_name;
	};

	struct Configuration
	{
		CoreConfiguration core_config;
	};
}
#endif