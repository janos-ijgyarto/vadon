#ifndef VADON_CORE_SYSTEM_ENGINESYSTEM_HPP
#define VADON_CORE_SYSTEM_ENGINESYSTEM_HPP
#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/System/System.hpp>
namespace Vadon::Core
{
	class EngineSystemBase : public Logger
	{
	public:
		void log(std::string_view message) override { m_engine_core.get_logger().log(message); }
		void warning(std::string_view message) override { m_engine_core.get_logger().warning(message); }
		void error(std::string_view message) override { m_engine_core.get_logger().error(message); }
	protected:
		EngineSystemBase(EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{
		}

		EngineCoreInterface& m_engine_core;
	};

	template<typename Module, typename SysImpl>
	class EngineSystem : public EngineSystemBase, public System<Module, SysImpl>
	{
	protected:
		EngineSystem(EngineCoreInterface& core)
			: EngineSystemBase(core)
			, System<Module, SysImpl>(core)
		{
		}
	};
}
#endif