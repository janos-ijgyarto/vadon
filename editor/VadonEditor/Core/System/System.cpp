#include <VadonEditor/Core/System/System.hpp>

#include <Vadon/Core/CoreInterface.hpp>

namespace VadonEditor::Core
{
	void SystemBase::log_message(std::string_view message) const { m_editor.get_engine_core().log_message(message); }
	void SystemBase::log_warning(std::string_view message) const { m_editor.get_engine_core().log_warning(message); }
	void SystemBase::log_error(std::string_view message) const { m_editor.get_engine_core().log_error(message); }
}