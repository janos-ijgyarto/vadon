#ifndef VADONEDITOR_MODEL_MODULE_HPP
#define VADONEDITOR_MODEL_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/System/Module.hpp>
namespace VadonEditor::Model
{
	class ModelModule : public Core::SystemModule<Core::SystemModuleList, ModelModule, class ModelSystem>
	{
	};

	template<typename SysImpl>
	using ModelSystemBase = Core::System<ModelModule, SysImpl>;
}
#endif