#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Core/Logger.hpp>

#include <Vadon/Utilities/TypeInfo/TypeList.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <format>

namespace Vadon::Utilities
{
	namespace
	{
		// FIXME: create proper serialization API!
		template <typename T>
		concept IsTrivialJSONWrite = Vadon::Utilities::IsAnyOf<T, int, float, bool, std::string>;

		class BinarySerializer : public Serializer
		{
		public:
			// TODO: binary serialization
			// - Combination of field tables and one large raw data buffer
			// - Field tables are key-value lookups, key leads to offset into raw data
			// - Arrays: each index stores offset to data (expect client code to know what should be there)
			// - Only push to raw data once we are actually writing raw values, otherwise push to field tables (hierarchical)
			//		- This should enable streaming
			// - Use a string table to save on size
			BinarySerializer(std::vector<std::byte>& buffer, Mode mode)
				: Serializer(buffer, mode)
			{
			}

			bool initialize() override
			{
				// TODO
				return false;
			}

			bool finalize() override
			{
				// TODO
				return false;
			}

			Result close_array() override { return Result::NOT_IMPLEMENTED; }
			Result close_object() override { return Result::NOT_IMPLEMENTED; }

			bool has_key(std::string_view /*key*/) const override { return false; }
		protected:
			Result serialize_int(int& /*value*/) override { return Result::NOT_IMPLEMENTED; }
			Result serialize_float(float& /*value*/) override { return Result::NOT_IMPLEMENTED; }
			Result serialize_bool(bool& /*value*/) override { return Result::NOT_IMPLEMENTED; }
			Result serialize_string(std::string& /*value*/) override { return Result::NOT_IMPLEMENTED; }

			Result set_value_reference(std::string_view /*key*/) override { return Result::NOT_IMPLEMENTED; }
			Result set_value_reference(size_t /*index*/) override { return Result::NOT_IMPLEMENTED; }

			Result internal_open_object() override { return Result::NOT_IMPLEMENTED; }
			Result internal_open_array() override { return Result::NOT_IMPLEMENTED; }

			size_t get_array_size() const override
			{
				// TODO
				return 0;
			}
		};

		class JSONSerializer : public Serializer
		{
		public:
			using Iterator = JSON::iterator;

			JSONSerializer(std::vector<std::byte>& buffer, Mode mode)
				: Serializer(buffer, mode)
			{
			}

			bool initialize() override
			{
				if (is_reading() == true)
				{
					try
					{
						m_json_root = JSON::parse(m_buffer);
					}
					catch (JSON::parse_error& exception)
					{
						Vadon::Core::Logger::log_error(std::format("Error parsing JSON data: {}\nException ID: {}\nError byte position: {}\n", exception.what(), exception.id, exception.byte));
						return false;
					}
				}
				else
				{
					// Start with empty root object
					m_json_root = JSON::object();
				}
				return true;
			}

			bool finalize() override
			{
				if (is_reading() == true)
				{
					// TODO: clear memory?
				}
				else
				{
					// Dump JSON to buffer
					// FIXME: is this the right way to do it?
					const std::string json_dump = m_json_root.dump(4);
					m_buffer.resize(json_dump.size());
					memcpy(m_buffer.data(), json_dump.data(), json_dump.size());
				}

				return true;
			}
			
			Result close_array() override
			{
				// Only allow if we were in an array to begin with
				JSON& current_object = get_current_value();
				if (current_object.is_array() == false)
				{
					return Result::INVALID_CONTAINER;
				}
				if (pop_iterator_stack() == false)
				{
					return Result::INVALID_CONTAINER;
				}
				return Result::SUCCESSFUL;
			}

			Result close_object() override
			{
				// Only allow if we were in an object to begin with
				JSON& current_object = get_current_value();
				if (current_object.is_object() == false)
				{
					return Result::INVALID_CONTAINER;
				}
				if (pop_iterator_stack() == false)
				{
					return Result::INVALID_CONTAINER;
				}
				return Result::SUCCESSFUL;
			}

			bool has_key(std::string_view key) const override
			{
				// Only allow if we were in an object to begin with
				const JSON& current_object = get_current_value();
				if (current_object.is_object() == true)
				{
					return current_object.find(key) != current_object.end();
				}
				return false;
			}
		protected:
			Result set_value_reference(std::string_view key) override
			{
				// Can only set by string key in an object
				JSON& current_value = get_current_value();
				if (current_value.is_object() == false)
				{
					return Result::INVALID_CONTAINER;
				}

				m_value_it = current_value.find(key);
				if (m_value_it == current_value.end())
				{
					// When reading, make sure key exists!
					if (is_reading() == true)
					{
						return Result::INVALID_KEY;
					}
					else
					{
						m_value_it = current_value.emplace(key, 0).first;
					}
				}				

				return Result::SUCCESSFUL;
			}

			Result set_value_reference(size_t index) override
			{
				// Can only set by index in an array
				JSON& current_value = get_current_value();
				if (current_value.is_array() == false)
				{
					return Result::INVALID_CONTAINER;
				}

				// Check bounds
				if (is_reading() == true)
				{
					if (index >= current_value.size())
					{
						return Result::INVALID_KEY;
					}
				}
				else
				{
					// When writing, setting the end index is equal to "push back"
					if (index == current_value.size())
					{
						current_value.emplace_back();
					}
					else if (index > current_value.size())
					{
						return Result::INVALID_KEY;
					}
				}

				m_value_it = current_value.begin() + index;
				return Result::SUCCESSFUL;
			}

			Result internal_open_object() override 
			{ 
				if (is_reading() == true)
				{
					if (m_value_it->is_object() == false)
					{
						return Result::INVALID_CONTAINER;
					}
				}
				else
				{
					*m_value_it = JSON::object();
				}
				push_iterator_stack(m_value_it);
				return Result::SUCCESSFUL;			
			}
			Result internal_open_array() override 
			{
				if (is_reading() == true)
				{
					if (m_value_it->is_array() == false)
					{
						return Result::INVALID_CONTAINER;
					}
				}
				else
				{
					*m_value_it = JSON::array();
				}
				push_iterator_stack(m_value_it);
				return Result::SUCCESSFUL;
			}

			Result serialize_int(int& value) override { return serialize_trivial(value); }
			Result serialize_float(float& value) override { return serialize_trivial(value); }
			Result serialize_bool(bool& value) override { return serialize_trivial(value); }
			Result serialize_string(std::string& value) override { return serialize_trivial(value); }

			template<typename T>
			Result serialize_trivial(T& value)
			{
				// TODO: have some way to catch if parsing was incorrect?
				if (is_reading() == true)
				{
					value = m_value_it->get<T>();
				}
				else
				{
					*m_value_it = value;
				}
				return Result::SUCCESSFUL;
			}

			size_t get_array_size() const override
			{
				const JSON& current_value = get_current_value();
				if (current_value.is_array() == true)
				{
					return current_value.size();
				}
				return 0;
			}

			const JSON& get_current_value() const
			{
				return m_iterator_stack.empty() ? m_json_root : *m_iterator_stack.back();
			}

			JSON& get_current_value()
			{
				return const_cast<JSON&>(std::as_const(*this).get_current_value());
			}

			void push_iterator_stack(Iterator iterator)
			{
				m_iterator_stack.push_back(iterator);
			}

			bool pop_iterator_stack()
			{
				if (m_iterator_stack.empty() == false)
				{
					m_iterator_stack.pop_back();
					return true;
				}

				return false;
			}

			JSON m_json_root;
			std::vector<Iterator> m_iterator_stack;
			Iterator m_value_it;
		};
	}

	Serializer::Instance Serializer::create_serializer(std::vector<std::byte>& buffer, Type type, Mode mode)
	{
		switch (type)
		{
		case Type::BINARY:
			return Serializer::Instance(new BinarySerializer(buffer, mode));
		case Type::JSON:
			return Serializer::Instance(new JSONSerializer(buffer, mode));
		}
		return nullptr;
	}

	Serializer::Result Serializer::open_array(std::string_view key)
	{
		const Result key_result = set_value_reference(key);
		if (key_result == Result::SUCCESSFUL)
		{
			return internal_open_array();
		}

		return key_result;
	}

	Serializer::Result Serializer::open_array(size_t index)
	{
		const Result index_result = set_value_reference(index);
		if (index_result == Result::SUCCESSFUL)
		{
			return internal_open_array();
		}

		return index_result;
	}

	Serializer::Result Serializer::open_object(std::string_view key)
	{
		const Result key_result = set_value_reference(key);
		if (key_result == Result::SUCCESSFUL)
		{
			return internal_open_object();
		}

		return key_result;
	}

	Serializer::Result Serializer::open_object(size_t index)
	{
		const Result index_result = set_value_reference(index);
		if (index_result == Result::SUCCESSFUL)
		{
			return internal_open_object();
		}

		return index_result;
	}

	Serializer::Result Serializer::serialize_uuid(Vadon::Utilities::UUID& value)
	{
		std::string base64_uuid;
		if (is_reading() == true)
		{
			const Result parse_result = internal_serialize_value(base64_uuid);
			if (parse_result != Result::SUCCESSFUL)
			{
				return parse_result;
			}
			if (value.from_base64_string(base64_uuid) == false)
			{
				return Result::INVALID_DATA;
			}
		}
		else
		{
			base64_uuid = value.to_base64_string();
			return internal_serialize_value(base64_uuid);
		}

		return Result::SUCCESSFUL;
	}

	Serializer::Serializer(std::vector<std::byte>& buffer, Mode mode)
		: m_mode(mode)
		, m_buffer(buffer)
	{
	}
}