#ifndef VADONEDITOR_RENDER_MODULE_HPP
#define VADONEDITOR_RENDER_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/System/Module.hpp>
namespace VadonEditor::Render
{
	class RenderModule : public Core::SystemModule<Core::SystemModuleList, RenderModule, class RenderSystem>
	{
	};

	template<typename SysImpl>
	using RenderSystemBase = Core::System<RenderModule, SysImpl>;
}
#endif