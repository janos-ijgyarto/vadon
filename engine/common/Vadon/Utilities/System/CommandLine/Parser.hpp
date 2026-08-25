#ifndef VADON_UTILITIES_SYSTEM_COMMANDLINE_PARSER_HPP
#define VADON_UTILITIES_SYSTEM_COMMANDLINE_PARSER_HPP
#include <string>
#include <unordered_map>
namespace Vadon::Utilities
{
	class CommandLineParser
	{
	public:
		// TODO: add ability to register options and other metadata
		// For now, just use a simple convention and store as raw strings
		void parse(int argc, char* argv[]);

		std::string_view get_entry(std::string_view key) const;
		bool has_entry(std::string_view key) const;
	private:
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_arguments;
	};
}
#endif