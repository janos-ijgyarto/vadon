#ifndef VADON_UTILITIES_SERIALIZATION_JSON_JSON_HPP
#define VADON_UTILITIES_SERIALIZATION_JSON_JSON_HPP
#include <nlohmann/json_fwd.hpp>
#include <string>
namespace Vadon::Utilities
{
	using JSON = nlohmann::json;
	using OrderedJSON = nlohmann::ordered_json;

	// Utility object to assist with deserialization
	class JSONReader
	{
	public:
		JSONReader(const JSON& root);
		~JSONReader();

		bool open_object(std::string_view key);
		bool open_array(std::string_view key);
		void close_object();

		const JSON& get_current_object() const;
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif