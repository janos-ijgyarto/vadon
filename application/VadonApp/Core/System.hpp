#ifndef VADONAPP_CORE_SYSTEM_HPP
#define VADONAPP_CORE_SYSTEM_HPP
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/System/System.hpp>
#include <VadonApp/Core/Application.hpp>
namespace VadonApp::Core
{
	// Utility class, makes creating app subsystems more concise
	// All app systems are implicitly engine system extensions
	// App systems can also access the app for additional features
	class SystemBase : public Vadon::Core::Logger
	{
	public:
		void log(std::string_view message) override;
		void warning(std::string_view message) override;
		void error(std::string_view message) override;
	protected:
		SystemBase(Application& application)
			: m_application(application)
		{}

		// FIXME: this is a hack, should have a proper way to give access to the registry!
		Vadon::Utilities::SingletonRegistry& get_registry() { return m_application.m_singleton_registry; }

		Application& m_application;
	};

	template<typename Module, typename SysImpl>
	class System : public SystemBase, public Vadon::Utilities::Singleton<Module, SysImpl>
	{
	protected:
		System(Application& application)
			: SystemBase(application)
			, Vadon::Utilities::Singleton<Module, SysImpl>(get_registry())
		{
		}
	};
}
#endif