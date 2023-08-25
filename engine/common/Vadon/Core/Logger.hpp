#ifndef VADON_CORE_LOGGER_HPP
#define VADON_CORE_LOGGER_HPP
#include <string>
namespace Vadon::Core
{
	class Logger
	{
	public:
		virtual void log(std::string_view message) = 0;
		virtual void warning(std::string_view message) = 0;
		virtual void error(std::string_view message) = 0;
	};
}
#endif