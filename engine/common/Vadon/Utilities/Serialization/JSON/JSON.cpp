#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSON.hpp>

#include <nlohmann/json.hpp>
namespace Vadon::Utilities
{
	namespace
	{
		using JSONReaderIterator = JSON::const_iterator;
	}

	struct JSONReader::Internal
	{
		const JSON& root;
		std::vector<JSONReaderIterator> iterator_stack;

		Internal(const JSON& root_json) : root(root_json) {}
	};

	JSONReader::JSONReader(const JSON& root)
		: m_internal(std::make_unique<Internal>(root))
	{
	}

	JSONReader::~JSONReader() = default;

	bool JSONReader::open_object(std::string_view key)
	{
		const JSON& current_object = get_current_object();

		JSONReaderIterator current_it = current_object.find(key);
		if ((current_it != current_object.end()) && (current_it->is_object() == true))
		{
			// Add the iterator to the stack (any subsequent reads will be directed to this sub-object)
			m_internal->iterator_stack.push_back(current_it);
			return true;
		}
		else
		{
			// No such sub-object exists, or it's not an object
			return false;
		}
	}

	bool JSONReader::open_array(std::string_view key)
	{
		const JSON& current_object = get_current_object();

		JSONReaderIterator current_it = current_object.find(key);
		if ((current_it != current_object.end()) && (current_it->is_array() == true))
		{
			// Add the iterator to the stack (any subsequent reads will be directed to this sub-object)
			m_internal->iterator_stack.push_back(current_it);
			return true;
		}
		else
		{
			// No such sub-object exists, or it's not an array
			return false;
		}
	}

	void JSONReader::close_object()
	{
		m_internal->iterator_stack.pop_back();
	}

	const JSON& JSONReader::get_current_object() const
	{
		return (m_internal->iterator_stack.empty() ? m_internal->root : (*m_internal->iterator_stack.back()));
	}
}