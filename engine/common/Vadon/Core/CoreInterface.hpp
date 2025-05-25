#ifndef VADON_CORE_COREINTERFACE_HPP
#define VADON_CORE_COREINTERFACE_HPP
#include <Vadon/Core/Configuration.hpp>
#include <Vadon/Core/System/SystemRegistry.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface : public SystemRegistry
	{
	public:
		virtual ~EngineCoreInterface() {}

		virtual const CommonConfiguration& get_config() const = 0;

		// TODO: implement interface (similar to system registry) that allows client code to register additional modules
		// This would allow them to access client-specific managers through the core interface
	};
}
#endif