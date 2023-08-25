#ifndef VADON_CORE_SYSTEM_SYSTEMMODULELIST_HPP
#define VADON_CORE_SYSTEM_SYSTEMMODULELIST_HPP
#include <Vadon/Utilities/Type/TypeList.hpp>
namespace Vadon::Render
{
	class RenderModule;
	class GraphicsModule;
}
namespace Vadon::Core
{
	using SystemModuleList = Vadon::Utilities::TypeList<class CoreModule, class Render::RenderModule, class Render::GraphicsModule>;
}
#endif