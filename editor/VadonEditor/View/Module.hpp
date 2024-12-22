#ifndef VADONEDITOR_VIEW_MODULE_HPP
#define VADONEDITOR_VIEW_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/System/Module.hpp>
namespace VadonEditor::View
{
	class ViewModule : public Core::SystemModule<Core::SystemModuleList, ViewModule, class ViewSystem>
	{
	};

	template<typename SysImpl>
	using ViewSystemBase = Core::System<ViewModule, SysImpl>;
}
#endif