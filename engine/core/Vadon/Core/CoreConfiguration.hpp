#ifndef VADON_CORE_CORECONFIGURATION_HPP
#define VADON_CORE_CORECONFIGURATION_HPP
#include <Vadon/Core/Configuration.hpp>
namespace Vadon::Core
{
	struct CoreConfiguration
	{
		CommonConfiguration common_config;
		// TODO: config for graphics API, etc.
	};
}
#endif