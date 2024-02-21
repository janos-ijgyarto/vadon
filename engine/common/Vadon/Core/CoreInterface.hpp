#ifndef VADON_CORE_COREINTERFACE_HPP
#define VADON_CORE_COREINTERFACE_HPP
#include <Vadon/Core/Configuration.hpp>
#include <Vadon/Core/System/SystemRegistry.hpp>
namespace Vadon::Core
{
	class Logger;

	class EngineCoreInterface : public SystemRegistry
	{
	public:
		virtual ~EngineCoreInterface() {}

		virtual bool initialize(const Configuration& config = Configuration()) = 0;
		virtual void update() = 0;
		virtual void shutdown() = 0;

		virtual const Configuration& get_config() const = 0;

		virtual void set_logger(Logger* logger) = 0;
		virtual Logger& get_logger() = 0;

		// TODO: implement interface (similar to system registry) that allows client code to register additional modules
		// This would allow them to access client-specific managers through the core interface
	};
}
#endif