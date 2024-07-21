#ifndef VADON_CORE_COREINTERFACE_HPP
#define VADON_CORE_COREINTERFACE_HPP
#include <Vadon/Core/Configuration.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/System/SystemRegistry.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface : public SystemRegistry, public LoggerInterface
	{
	public:
		virtual ~EngineCoreInterface() {}

		virtual bool initialize(const Configuration& config = Configuration()) = 0;
		virtual void update() = 0;
		virtual void shutdown() = 0;

		virtual const Configuration& get_config() const = 0;

		void log_message(std::string_view message) override { Logger::log_message(message); }
		void log_warning(std::string_view message) override { Logger::log_warning(message); }
		void log_error(std::string_view message) override { Logger::log_error(message); }

		// TODO: implement interface (similar to system registry) that allows client code to register additional modules
		// This would allow them to access client-specific managers through the core interface
	};
}
#endif