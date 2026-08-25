#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

namespace Vadon::Utilities
{
	void CommandLineParser::parse(int argc, char* argv[])
	{
		m_program_name = argv[0];

		std::string_view current_key = "";
		for (int32_t current_arg_index = 1; current_arg_index < argc; ++current_arg_index)
		{
            const std::string_view argument_string(argv[current_arg_index]);

			if (argument_string.substr(0, 2) == "--")
			{
				current_key = argument_string.substr(2);
				if (current_key.empty() == false)
				{
					if (has_entry(current_key) == true)
					{
						// TODO: error?
						current_key = "";
						continue;
					}

					// Add key to lookup, for now with an empty string
					m_arguments.insert(std::make_pair(std::string(current_key), ""));
				}
				else
				{
					// TODO: log error?
				}
				continue;
			}

			if (current_key.empty() == false)
			{
				// Overwrite the entry
				m_arguments[std::string(current_key)] = std::string(argument_string);

				// Reset key
				current_key = "";
			}
			else
			{
				// TODO: log error?
			}
		}
	}

	std::string_view CommandLineParser::get_entry(std::string_view key) const
	{
		auto entry_it = m_arguments.find(std::string(key));
		if (entry_it != m_arguments.end())
		{
			return entry_it->second;
		}

		return std::string_view();
	}

	bool CommandLineParser::has_entry(std::string_view key) const
	{
		return m_arguments.find(std::string(key)) != m_arguments.end();
	}
}