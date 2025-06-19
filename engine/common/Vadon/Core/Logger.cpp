#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Core/Environment.hpp>

#include <iostream>
#include <syncstream>

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

    // TODO: need some way to "tag" different messages so UI can highlight, etc.
    void DefaultLogger::log_message(std::string_view message)
    {
        std::osyncstream cout_sync(std::cout);
        cout_sync << message;
    }

    void DefaultLogger::log_warning(std::string_view message)
    {
        // TODO: timestamp?
        std::osyncstream cout_sync(std::cout);
        cout_sync << "WARNING: " << message;
    }

    void DefaultLogger::log_error(std::string_view message)
    {
        // TODO: timestamp?
        std::osyncstream cerr_sync(std::cerr);
        cerr_sync << "ERROR: " << message;
    }
}