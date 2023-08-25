#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Core/System.hpp>

#include <VadonApp/Core/Application.hpp>

#include <Vadon/Core/CoreInterface.hpp>

namespace VadonApp::Core
{
	void SystemBase::log(std::string_view message) { m_application.get_engine_core().get_logger().log(message); }
	void SystemBase::warning(std::string_view message) { m_application.get_engine_core().get_logger().warning(message); }
	void SystemBase::error(std::string_view message) { m_application.get_engine_core().get_logger().error(message); }
}