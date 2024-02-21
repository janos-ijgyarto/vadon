#ifndef VADON_UTILITIES_SERIALIZATION_JSON_JSON_HPP
#define VADON_UTILITIES_SERIALIZATION_JSON_JSON_HPP
#include <Vadon/Common.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
namespace Vadon::Utilities
{
	using JSON = nlohmann::json;
	using OrderedJSON = nlohmann::ordered_json;

	// Utility objects to assist with deserialization
	class JSONReader
	{
	public:
		VADONCOMMON_API JSONReader(const JSON& root);
		VADONCOMMON_API ~JSONReader();

		VADONCOMMON_API bool open_object(std::string_view key);
		VADONCOMMON_API bool open_array(std::string_view key);
		VADONCOMMON_API void close_object();

		VADONCOMMON_API const JSON& get_current_object() const;
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif