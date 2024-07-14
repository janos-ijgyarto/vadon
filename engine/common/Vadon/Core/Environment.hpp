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
	// Utility object that contains globals, explicitly initialized at startup
	// Enables global data for specific use cases (e.g type registry) while 
	// ensuring that we still keep initialization explicit
	struct EngineEnvironment
	{
		VADONCOMMON_API EngineEnvironment();
		VADONCOMMON_API ~EngineEnvironment();

		std::unique_ptr<::Vadon::ECS::ComponentRegistry> component_registry;
		std::unique_ptr<::Vadon::Utilities::TypeRegistry> type_registry;

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

		static EngineEnvironment* s_instance;
	};
}
#endif