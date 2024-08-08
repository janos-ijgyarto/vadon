#ifndef VADON_CORE_LOGGER_HPP
#define VADON_CORE_LOGGER_HPP
#include <Vadon/Common.hpp>
#include <string>
namespace Vadon::Core
{
	class Logger
	{
	public:
		VADONCOMMON_API static void log_message(std::string_view message);
		VADONCOMMON_API static void log_warning(std::string_view message);
		VADONCOMMON_API static void log_error(std::string_view message);
	};

	class LoggerInterface
	{
	public:
		virtual void log_message(std::string_view message) const = 0;
		virtual void log_warning(std::string_view message) const = 0;
		virtual void log_error(std::string_view message) const = 0;
	};
}
#endif