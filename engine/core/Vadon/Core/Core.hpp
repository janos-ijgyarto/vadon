#ifndef VADON_CORE_CORE_HPP
#define VADON_CORE_CORE_HPP
#include <Vadon/Core.hpp>
#include <Vadon/Core/CoreInterface.hpp>
#include <memory>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace Vadon::Core
{
	class EngineEnvironment;
	struct CoreConfiguration;

	class EngineCoreImplementation : public EngineCoreInterface
	{
	public:
		virtual bool initialize(const CoreConfiguration& config) = 0;
		virtual void shutdown() = 0;
	};

	using EngineCorePtr = std::unique_ptr<EngineCoreImplementation>;

	VADONCORE_API void init_engine_environment(EngineEnvironment& environment);
	VADONCORE_API void register_engine_types();
	VADONCORE_API void register_engine_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	VADONCORE_API EngineCorePtr create_engine_core();
}
#endif