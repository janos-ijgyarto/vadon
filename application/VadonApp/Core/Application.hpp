#ifndef VADONAPP_CORE_APPLICATION_HPP
#define VADONAPP_CORE_APPLICATION_HPP
#include <VadonApp/VadonApp.hpp>
#include <VadonApp/Core/Configuration.hpp>
#include <VadonApp/Core/SystemRegistry.hpp>

#include <Vadon/Core/CoreInterface.hpp>

namespace Vadon::Core
{
	class EngineEnvironment;
}

namespace VadonApp::Core
{
	class SystemBase;

	class Application : public SystemRegistry, public Vadon::Core::LoggerInterface
	{
	public:
		using Instance = std::unique_ptr<Application>;

		virtual ~Application() {}

		virtual bool initialize(const Configuration& config = Configuration()) = 0;
		virtual void update() = 0; // FIXME: currently just updates the platform, might want to revise and use tasks & event buffering
		virtual void shutdown() = 0;

		virtual const Vadon::Core::EngineCoreInterface& get_engine_core() const = 0;
		Vadon::Core::EngineCoreInterface& get_engine_core() { return const_cast<Vadon::Core::EngineCoreInterface&>(std::as_const(*this).get_engine_core()); }

		virtual const Configuration& get_config() const = 0;

		void log_message(std::string_view message) const override { get_engine_core().log_message(message); }
		void log_warning(std::string_view message) const override { get_engine_core().log_warning(message); }
		void log_error(std::string_view message) const override { get_engine_core().log_error(message); }

		// TODO: make environment extended for app?
		static VADONAPP_API void init_application_environment(Vadon::Core::EngineEnvironment& environment);
		static VADONAPP_API Instance create_instance();
	};
}
#endif