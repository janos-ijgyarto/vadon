#ifndef VADONAPP_CORE_APPLICATION_HPP
#define VADONAPP_CORE_APPLICATION_HPP
#include <VadonApp/VadonApp.hpp>
#include <VadonApp/Core/Configuration.hpp>
#include <VadonApp/Core/SystemRegistry.hpp>

namespace Vadon::Core
{
	class EngineCoreInterface;
	struct EngineEnvironment;
}

namespace VadonApp::Core
{
	class SystemBase;

	class Application : public SystemRegistry
	{
	public:
		using Instance = std::unique_ptr<Application>;

		virtual ~Application() {}

		virtual bool initialize(const Configuration& config = Configuration()) = 0;
		virtual void update() = 0; // FIXME: currently just updates the platform, might want to revise and use tasks & event buffering
		virtual void shutdown() = 0;

		virtual Vadon::Core::EngineCoreInterface& get_engine_core() = 0;

		virtual const Configuration& get_config() const = 0;

		// TODO: make environment extended for app?
		static VADONAPP_API void init_application_environment(Vadon::Core::EngineEnvironment& environment);
		static VADONAPP_API Instance create_instance();
	};
}
#endif