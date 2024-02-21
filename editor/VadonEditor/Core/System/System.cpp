#include <VadonEditor/Core/System/System.hpp>

#include <Vadon/Core/CoreInterface.hpp>

namespace VadonEditor::Core
{
	void SystemBase::log(std::string_view message) { m_editor.get_engine_core().get_logger().log(message); }
	void SystemBase::warning(std::string_view message) { m_editor.get_engine_core().get_logger().warning(message); }
	void SystemBase::error(std::string_view message) { m_editor.get_engine_core().get_logger().error(message); }
}