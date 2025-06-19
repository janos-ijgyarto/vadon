#ifndef VADON_CORE_CONFIGURATION_HPP
#define VADON_CORE_CONFIGURATION_HPP
namespace Vadon::Core
{
	struct CommonConfiguration
	{
		// TODO: settings which need to be applied when "initialize" is called
		// This is mostly relevant for subsystems which must be created in this phase, e.g selecting graphics backend
		// Afterward, client code can directly initialize specific subsystems (e.g starting up task threads)
	};
}
#endif