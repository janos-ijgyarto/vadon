#ifndef VADONDEMO_CORE_CORE_HPP
#define VADONDEMO_CORE_CORE_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace VadonDemo::Core
{
	// TODO: have this class contain the other systems?
	class Core
	{
	public:
		VADONDEMO_API Core(Vadon::Core::EngineEnvironment& environment);
		VADONDEMO_API static void register_types();
	};
}
#endif