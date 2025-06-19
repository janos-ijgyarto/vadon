#ifndef VADONAPP_CORE_APPLICATION_HPP
#define VADONAPP_CORE_APPLICATION_HPP
#include <VadonApp/VadonApp.hpp>
#include <VadonApp/Core/Configuration.hpp>
#include <VadonApp/Core/SystemRegistry.hpp>

#include <unordered_map>

namespace Vadon::Core
{
	class EngineCoreInterface;
	class EngineEnvironment;
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
		virtual void shutdown() = 0;

		virtual const Configuration& get_config() const = 0;

		Vadon::Core::EngineCoreInterface& get_engine_core() { return m_engine_core; }

		// TODO: make environment extended for app?
		static VADONAPP_API void init_application_environment(Vadon::Core::EngineEnvironment& environment);
		static VADONAPP_API Instance create_instance(Vadon::Core::EngineCoreInterface& engine_core);

		// FIXME: implement a proper CLI parser!
		VADONAPP_API void parse_command_line(int argc, char* argv[]);
		VADONAPP_API bool has_command_line_arg(std::string_view name) const;
		VADONAPP_API std::string get_command_line_arg(std::string_view name) const;
	protected:
		Application(Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{ }

		void parse_command_line_argument(const char* argument_ptr);

		Vadon::Core::EngineCoreInterface& m_engine_core;

		// FIXME: implement a proper CLI parser!
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_command_line_args;
	};
}
#endif