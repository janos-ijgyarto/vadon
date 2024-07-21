#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Core/Environment.hpp>

namespace Vadon::Core
{
	void Logger::log_message(std::string_view message)
	{
		Vadon::Core::EngineEnvironment::get_logger().log_message(message);
	}

	void Logger::log_warning(std::string_view message)
	{
		Vadon::Core::EngineEnvironment::get_logger().log_warning(message);
	}

	void Logger::log_error(std::string_view message)
	{
		Vadon::Core::EngineEnvironment::get_logger().log_error(message);
	}
}