#ifndef VADON_CORE_SYSTEM_SYSTEM_HPP
#define VADON_CORE_SYSTEM_SYSTEM_HPP
#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Utilities/System/Singleton/Singleton.hpp>
namespace Vadon::Core
{
	class SystemBase : public Logger
	{
	public:
		void log(std::string_view message) override { m_engine_core.get_logger().log(message); }
		void warning(std::string_view message) override { m_engine_core.get_logger().warning(message); }
		void error(std::string_view message) override { m_engine_core.get_logger().error(message); }
	protected:
		SystemBase(EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{
		}

		// FIXME: this is a hack, should have a proper way to give access to the registry!
		Utilities::SingletonRegistry& get_registry() { return m_engine_core.m_singleton_registry; }

		EngineCoreInterface& m_engine_core;
	};
			
	template<typename Module, typename SysImpl>
	class System : public SystemBase, public Utilities::Singleton<Module, SysImpl>
	{
	protected:
		System(EngineCoreInterface& core)
			: SystemBase(core)
			, Utilities::Singleton<Module, SysImpl>(get_registry())
		{
		}
	};
}
#endif