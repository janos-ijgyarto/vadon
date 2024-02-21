#ifndef VADONEDITOR_UI_MODULE_HPP
#define VADONEDITOR_UI_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/System/Module.hpp>
namespace VadonEditor::UI
{
	class UIModule : public Core::SystemModule<Core::SystemModuleList, UIModule, class UISystem>
	{
	};

	template<typename SysImpl>
	using UISystemBase = Core::System<UIModule, SysImpl>;
}
#endif