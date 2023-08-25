#ifndef VADON_RENDER_RENDERMODULE_HPP
#define VADON_RENDER_RENDERMODULE_HPP
#include <Vadon/Core/System/EngineSystem.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
#include <Vadon/Core/System/SystemModuleList.hpp>
namespace Vadon::Render
{
	class RenderModule : public Core::SystemModule<Core::SystemModuleList, RenderModule, class FrameSystem>
	{
	};

	template<typename SysImpl>
	using RenderSystem = Core::EngineSystem<RenderModule, SysImpl>;
}
#endif