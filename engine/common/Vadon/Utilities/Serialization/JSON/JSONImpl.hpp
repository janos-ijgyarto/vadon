#ifndef VADON_UTILITIES_SERIALIZATION_JSON_JSONIMPL_HPP
#define VADON_UTILITIES_SERIALIZATION_JSON_JSONIMPL_HPP
#include <Vadon/Utilities/Serialization/JSON/JSON.hpp>
#include <nlohmann/json.hpp>

namespace Vadon::Utilities
{
	using JSONValueType = nlohmann::json::value_t;
}
#endif