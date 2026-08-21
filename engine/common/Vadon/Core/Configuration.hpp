#ifndef VADON_CORE_CONFIGURATION_HPP
#define VADON_CORE_CONFIGURATION_HPP
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Core
{
	enum class RenderConfigurationFlags
	{
		NONE = 0,
		DISABLE_RENDERING = 1 << 0
		// TODO: other flags?
	};

	struct RenderConfiguration
	{
		RenderConfigurationFlags flags = RenderConfigurationFlags::NONE;
		// TODO: other settings?
	};

	struct CommonConfiguration
	{
		// TODO: settings which need to be applied when "initialize" is called
		// This is mostly relevant for subsystems which must be created in this phase, e.g selecting graphics backend
		// Afterward, client code can directly initialize specific subsystems (e.g starting up task threads)
		RenderConfiguration render_config;
	};
}

VADON_ENABLE_BITWISE_OPERATORS(Vadon::Core::RenderConfigurationFlags);
#endif