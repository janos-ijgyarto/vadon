#ifndef VADON_CORE_ENVIRONMENT_HPP
#define VADON_CORE_ENVIRONMENT_HPP
#include <Vadon/Common.hpp>
#include <memory>
namespace Vadon::ECS
{
	class ComponentRegistry;
}
namespace Vadon::Utilities
{
	class TypeRegistry;
}
namespace Vadon::Core
{
	class LoggerInterface;

	// Utility object that contains globals, explicitly initialized at startup
	// Enables global data for specific use cases (e.g type registry) while 
	// ensuring that we still keep initialization explicit
	class EngineEnvironment
	{
	public:
		VADONCOMMON_API EngineEnvironment();
		VADONCOMMON_API ~EngineEnvironment();

		static ::Vadon::ECS::ComponentRegistry& get_component_registry();
		static ::Vadon::Utilities::TypeRegistry& get_type_registry();
		static LoggerInterface& get_logger();

		VADONCOMMON_API static void set_logger(LoggerInterface* logger);

		// NOTE: the engine expects that this object is instantiated once immediately at startup
		// and the instance is passed to the appropriate initialization functions to ensure that
		// all binary modules point to the same instance, regardless of what linkage is used.
		// 
		// If DLLs are loaded dynamically, they also need an entrypoint where this instance can be
		// passed in.
		//
		// Multiple instances of the engine can exist and share the same environment, but only one
		// environment instance should be registered.
		static VADONCOMMON_API void initialize(EngineEnvironment& instance);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;

		static EngineEnvironment* s_instance;
	};
}
#endif