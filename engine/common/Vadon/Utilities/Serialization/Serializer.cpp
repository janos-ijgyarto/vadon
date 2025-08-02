#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Core/Logger.hpp>

#include <Vadon/Utilities/TypeInfo/TypeList/TypeList.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <format>

namespace Vadon::Utilities
{
	namespace
	{
		uint32_t reverse_uint_bytes(uint32_t input)
		{
			uint32_t output = 0;

			uint8_t* input_bytes = reinterpret_cast<uint8_t*>(&input);
			uint8_t* output_bytes = reinterpret_cast<uint8_t*>(&output);

			output_bytes[0] = input_bytes[3];
			output_bytes[1] = input_bytes[2];
			output_bytes[2] = input_bytes[1];
			output_bytes[3] = input_bytes[0];

			return output;
		}

		// FIXME: create proper serialization API!
		template <typename T>
		concept IsTrivialJSONWrite = Vadon::Utilities::IsAnyOf<T, int, float, bool, std::string>;

		class BinarySerializer : public Serializer
		{
		public:
			BinarySerializer(std::vector<std::byte>& buffer, Mode mode)
				: Serializer(buffer, mode)
			{
			}

			bool initialize() override
			{
				Object& root_object = m_object_stack.emplace_back();
				m_root_reference.type = ReferenceType::OBJECT;
				m_root_reference.stack_index = 0;

				if (is_reading() == true)
				{
					// First uint of buffer is the offset to the root metadata
					const uint32_t* data_ptr = reinterpret_cast<const uint32_t*>(m_buffer.data());
					parse_object_metadata(data_ptr[0], root_object);

					read_string_table();
				}
				else
				{
					// Add room for metadata
					// First uint: offset to root object data
					// Second uint: offset to string table data
					m_buffer.insert(m_buffer.end(), 2 * sizeof(uint32_t), std::byte{ 0 });
				}

				return true;
			}

			bool finalize() override
			{
				VADON_ASSERT((m_reference_stack.empty() == true) && (m_object_stack.size() == 1), "Invalid serializer state!");

				if (is_reading() == false)
				{
					// Write the root data offset to the first uint
					uint32_t* data_ptr = reinterpret_cast<uint32_t*>(m_buffer.data());
					*data_ptr = static_cast<uint32_t>(m_buffer.size());

					write_object_metadata(m_object_stack.front());
					write_string_table();
				}
				else
				{
					// TODO: anything?
				}

				m_finalized = true;
				return true;
			}

			void read_string_table()
			{
				m_string_table.clear();
				m_string_lookup.clear();

				const uint32_t* data_ptr = reinterpret_cast<const uint32_t*>(m_buffer.data());
				const uint32_t string_table_data_offset = data_ptr[1];

				const std::byte* string_table_ptr = m_buffer.data() + string_table_data_offset;
				const uint32_t string_table_size = *reinterpret_cast<const uint32_t*>(string_table_ptr);

				string_table_ptr += sizeof(uint32_t);

				for (uint32_t index = 0; index < string_table_size; ++index)
				{
					const uint32_t* string_entry_ptr = reinterpret_cast<const uint32_t*>(string_table_ptr);
					const uint32_t string_id = string_entry_ptr[0];
					const uint32_t string_length = string_entry_ptr[1];

					const char* start_ptr = reinterpret_cast<const char*>(string_table_ptr + (sizeof(uint32_t) * 2));
					std::string string_data(start_ptr, string_length);

					m_string_table.insert(std::make_pair(string_data, string_id));
					m_string_lookup.insert(std::make_pair(string_id, string_data));

					string_table_ptr += (sizeof(uint32_t) * 2) + string_length;
				}
			}

			void write_string_table()
			{
				// Write the string table offset to the second uint
				uint32_t* data_ptr = reinterpret_cast<uint32_t*>(m_buffer.data());
				data_ptr[1] = static_cast<uint32_t>(m_buffer.size());

				// Add the number of entries in the table
				const size_t table_size_offset = m_buffer.size();
				m_buffer.insert(m_buffer.end(), sizeof(uint32_t), std::byte{ 0 });
				
				reinterpret_cast<uint32_t*>(m_buffer.data() + table_size_offset)[0] = static_cast<uint32_t>(m_string_table.size());

				for (const auto& current_entry : m_string_table)
				{
					// For each entry, write the ID, the length, and the characters
					const size_t string_data_offset = m_buffer.size();
					m_buffer.insert(m_buffer.end(), 2 * sizeof(uint32_t) + current_entry.first.length(), std::byte{ 0 });

					uint32_t* string_data_ptr = reinterpret_cast<uint32_t*>(m_buffer.data() + string_data_offset);
					string_data_ptr[0] = current_entry.second;
					string_data_ptr[1] = static_cast<uint32_t>(current_entry.first.length());

					memcpy(string_data_ptr + 2, current_entry.first.c_str(), current_entry.first.length());
				}
			}

			Result close_array() override 
			{
				if (get_parent_ref().type != ReferenceType::ARRAY)
				{
					// We should be inside an object!
					return Result::INVALID_CONTAINER;
				}

				// Pop the reference from the stack
				pop_reference_stack();

				if (is_reading() == false)
				{
					const uint32_t array_metadata_offset = static_cast<uint32_t>(m_buffer.size());

					const ValueReference& parent_ref = get_parent_ref();
					if (parent_ref.type == ReferenceType::OBJECT)
					{
						// Write offset into entry in parent object
						Object& parent_object = m_object_stack[parent_ref.stack_index];

						auto value_it = parent_object.values.find(m_current_ref.value);
						if (value_it == parent_object.values.end())
						{
							// TODO: assert?
							return Result::INVALID_CONTAINER;
						}

						value_it->second = array_metadata_offset;
					}
					else if (parent_ref.type == ReferenceType::ARRAY)
					{
						// Write offset into entry in parent array
						Array& parent_array = m_array_stack[parent_ref.stack_index];

						if (m_current_ref.value >= parent_array.values.size())
						{
							// TODO: assert?
							return Result::INVALID_CONTAINER;
						}

						parent_array.values[m_current_ref.value] = array_metadata_offset;
					}
					else
					{
						// Something went wrong
						return Result::INVALID_CONTAINER;
					}

					const Array& current_array = m_array_stack[m_current_ref.stack_index];

					// Write the key-value pairs into the buffer
					m_buffer.insert(m_buffer.end(), (current_array.values.size() + 1) * sizeof(uint32_t), std::byte{ 0 });

					uint32_t* array_data = reinterpret_cast<uint32_t*>(m_buffer.data() + array_metadata_offset);
					*array_data = static_cast<uint32_t>(current_array.values.size());

					++array_data;
					memcpy(array_data, current_array.values.data(), current_array.values.size() * sizeof(uint32_t));
				}

				// Pop array from stack
				m_array_stack.pop_back();
				return Result::SUCCESSFUL;
			}
			
			Result close_object() override 
			{ 
				if (get_parent_ref().type != ReferenceType::OBJECT)
				{
					// We should be inside an object!
					return Result::INVALID_CONTAINER;
				}

				if (m_object_stack.empty() == true)
				{
					// Should not try to close root object!
					return Result::INVALID_CONTAINER;
				}

				// Pop the reference from the stack
				pop_reference_stack();

				if (is_reading() == false)
				{
					const uint32_t object_metadata_offset = static_cast<uint32_t>(m_buffer.size());

					const ValueReference& parent_ref = get_parent_ref();
					if (parent_ref.type == ReferenceType::OBJECT)
					{
						// Write offset into entry in parent object
						Object& parent_object = m_object_stack[parent_ref.stack_index];

						auto value_it = parent_object.values.find(m_current_ref.value);
						if (value_it == parent_object.values.end())
						{
							// TODO: assert?
							return Result::INVALID_CONTAINER;
						}

						value_it->second = object_metadata_offset;
					}
					else if (parent_ref.type == ReferenceType::ARRAY)
					{
						// Write offset into entry in parent array
						Array& parent_array = m_array_stack[parent_ref.stack_index];

						if (m_current_ref.value >= parent_array.values.size())
						{
							// TODO: assert?
							return Result::INVALID_CONTAINER;
						}

						parent_array.values[m_current_ref.value] = object_metadata_offset;
					}
					else
					{
						// Something went wrong
						return Result::INVALID_CONTAINER;
					}

					const Object& current_object = m_object_stack[m_current_ref.stack_index];
					write_object_metadata(current_object);
				}

				// Pop object from stack
				m_object_stack.pop_back();
				return Result::SUCCESSFUL; 
			}

			bool has_key(std::string_view key) const override
			{ 
				const ValueReference& parent_ref = get_parent_ref();
				VADON_ASSERT(parent_ref.type == ReferenceType::OBJECT, "Invalid container!");

				const uint32_t string_id = find_string_id(key);
				if (string_id == 0)
				{
					return false;
				}

				const Object& object = m_object_stack[parent_ref.stack_index];
				return object.values.find(string_id) != object.values.end();
			}
		protected:
			enum class ReferenceType
			{
				VALUE,
				OBJECT,
				ARRAY
			};

			struct ValueReference
			{
				ReferenceType type = ReferenceType::VALUE;
				size_t stack_index = 0;
				uint32_t value = 0; // NOTE: value when reading, key to write to when writing
			};

			struct Object
			{
				std::unordered_map<uint32_t, uint32_t> values;
			};

			struct Array
			{
				std::vector<uint32_t> values;
			};

			Result serialize_int(int& value) override { return serialize_direct(value); }
			Result serialize_float(float& value) override { return serialize_direct(value); }
			Result serialize_bool(bool& value) override 
			{ 
				Result result = Result::INVALID_DATA;
				if (is_reading() == true)
				{
					uint32_t bool_value = 0;
					result = serialize_direct(bool_value);
					if (result == Result::SUCCESSFUL)
					{
						value = bool_value != 0;
					}
				}
				else
				{
					uint32_t bool_value = value ? 1 : 0;
					result = serialize_direct(bool_value);
				}
				return result;
			}

			Result serialize_string(std::string& value) override
			{
				Result result = Result::INVALID_DATA;
				if (is_reading() == true)
				{
					uint32_t string_id = 0;
					result = serialize_direct(string_id);
					if (serialize_direct(string_id) == Result::SUCCESSFUL)
					{
						value = get_string(string_id);
					}
				}
				else
				{
					uint32_t string_id = get_string_id(value);
					result = serialize_direct(string_id);
				}
				return result;
			}
			
			Result serialize_color(Vadon::Math::ColorRGBA& color_value) override { return serialize_direct(color_value.value); }

			// TODO: find a way to generalize where we just provide a data pointer and a size?
			Result serialize_uuid(Vadon::Utilities::UUID& value) override
			{
				if (is_reading() == true)
				{
					const std::byte* data_ptr = m_buffer.data() + m_current_ref.value;
					memcpy(value.data.data(), data_ptr, sizeof(Vadon::Utilities::UUIDData));
				}
				else
				{
					const size_t prev_size = m_buffer.size();
					m_buffer.insert(m_buffer.end(), sizeof(Vadon::Utilities::UUIDData), std::byte{0});

					memcpy(m_buffer.data() + prev_size, value.data.data(), sizeof(Vadon::Utilities::UUIDData));

					write_value_entry(static_cast<uint32_t>(prev_size));
				}

				return Result::SUCCESSFUL;
			}

			void write_value_entry(uint32_t value)
			{
				VADON_ASSERT(is_reading() == false, "Invalid serializer state!");
				VADON_ASSERT(m_current_ref.type == ReferenceType::VALUE, "Invalid reference!");

				const ValueReference& parent_ref = get_parent_ref();
				if (parent_ref.type == ReferenceType::OBJECT)
				{
					// Write entry to parent object
					Object& parent_object = m_object_stack[parent_ref.stack_index];

					auto value_it = parent_object.values.find(m_current_ref.value);
					VADON_ASSERT(value_it != parent_object.values.end(), "Invalid key!");

					value_it->second = value;
				}
				else if (parent_ref.type == ReferenceType::ARRAY)
				{
					// Write entry to parent array
					Array& parent_array = m_array_stack[parent_ref.stack_index];
					VADON_ASSERT(m_current_ref.value < parent_array.values.size(), "Invalid index!");

					parent_array.values[m_current_ref.value] = value;
				}
				else
				{
					VADON_UNREACHABLE;
				}
			}

			// Direct: the value gets written directly into the object/array (must fit into uint32_t!)
			template<typename T>
			Result serialize_direct(T& value)
			{
				static_assert(sizeof(T) <= sizeof(uint32_t), "Invalid type!");

				// TODO: have some way to catch if parsing was incorrect?
				if (is_reading() == true)
				{
					value = std::bit_cast<T>(m_current_ref.value);
				}
				else
				{
					const uint32_t value_entry = std::bit_cast<uint32_t, T>(value);
					write_value_entry(value_entry);
				}
				return Result::SUCCESSFUL;
			}

			// Trivial: can simply serialize the value to/from the buffer
			// Entry in the parent container is the offset where we can find the data
			template<typename T>
			Result serialize_trivial(T& value)
			{
				// TODO: have some way to catch if parsing was incorrect?
				if (is_reading() == true)
				{
					value = *reinterpret_cast<T*>(m_buffer.data() + m_current_ref.value);
				}
				else
				{
					const size_t prev_size = m_buffer.size();
					m_buffer.insert(m_buffer.end(), sizeof(T), std::byte{ 0 });

					T* value_ptr = reinterpret_cast<T*>(m_buffer.data() + prev_size);
					*value_ptr = value;

					write_value_entry(static_cast<uint32_t>(prev_size));
				}
				return Result::SUCCESSFUL;
			}

			Result set_value_reference(std::string_view key) override 
			{
				const ValueReference& parent_ref = get_parent_ref();
				if (parent_ref.type != ReferenceType::OBJECT)
				{
					return Result::INVALID_CONTAINER;
				}

				const uint32_t key_id = get_string_id(key);
				if (key_id == 0)
				{
					return Result::INVALID_KEY;
				}

				Object& parent_object = m_object_stack[parent_ref.stack_index];
				auto object_key_it = parent_object.values.find(key_id);

				if (is_reading() == true)
				{
					// We are reading, so the key must be there!
					if (object_key_it == parent_object.values.end())
					{
						return Result::INVALID_KEY;
					}
				}
				else
				{
					// We are writing, should not try to overwrite existing key!
					if (object_key_it != parent_object.values.end())
					{
						return Result::INVALID_KEY;
					}

					// Set a dummy value for now
					parent_object.values[key_id] = 0;
				}

				// Set the value reference
				m_current_ref = ValueReference();

				if (is_reading() == true)
				{
					// When reading, the value in the ref is the entry in the object
					m_current_ref.value = object_key_it->second;
				}
				else
				{
					// When writing, the value in the ref is the key ID in the object (this is where the result will be written)
					m_current_ref.value = key_id;
				}

				return Result::SUCCESSFUL; 
			}

			Result set_value_reference(size_t index) override 
			{
				ValueReference& parent_ref = get_parent_ref();
				if (parent_ref.type != ReferenceType::ARRAY)
				{
					return Result::INVALID_CONTAINER;
				}

				Array& parent_array = m_array_stack[parent_ref.stack_index];

				// Check bounds
				if (is_reading() == true)
				{
					if (index >= parent_array.values.size())
					{
						return Result::INVALID_KEY;
					}
				}
				else
				{
					// When writing, setting the end index is equal to "push back"
					if (index == parent_array.values.size())
					{
						parent_array.values.push_back(0);
					}
					else if (index > parent_array.values.size())
					{
						return Result::INVALID_KEY;
					}
				}

				// Set the value reference
				m_current_ref = ValueReference();

				if (is_reading() == true)
				{
					// When reading, the value in the ref is the entry in the array
					m_current_ref.value = parent_array.values[index];
				}
				else
				{
					// When writing, the value in the ref is the index in the array
					m_current_ref.value = static_cast<uint32_t>(index);
				}

				return Result::SUCCESSFUL; 
			
			}

			Result internal_open_object() override 
			{
				if (m_current_ref.type != ReferenceType::VALUE)
				{
					return Result::INVALID_CONTAINER;
				}

				// Set the reference type and stack index
				m_current_ref.type = ReferenceType::OBJECT;
				m_current_ref.stack_index = m_object_stack.size();

				// Push reference to stack
				m_reference_stack.push_back(m_current_ref);

				// Add object to stack
				Object& new_object = m_object_stack.emplace_back();

				if (is_reading() == true)
				{
					parse_object_metadata(m_current_ref.value, new_object);
				}
				
				return Result::SUCCESSFUL;
			}

			void parse_object_metadata(uint32_t data_offset, Object& object)
			{
				object.values.clear();
				const uint32_t* data_ptr = reinterpret_cast<const uint32_t*>(m_buffer.data() + data_offset);

				const uint32_t object_table_size = *data_ptr;
				++data_ptr;

				for (uint32_t index = 0; index < object_table_size; ++index)
				{
					object.values.insert(std::make_pair(data_ptr[0], data_ptr[1]));
					data_ptr += 2;
				}
			}

			void write_object_metadata(const Object& object)
			{
				// Allocate space in the buffer
				const size_t data_offset = m_buffer.size();
				m_buffer.insert(m_buffer.end(), ((object.values.size() * 2) + 1) * sizeof(uint32_t), std::byte{ 0 });

				uint32_t* object_data = reinterpret_cast<uint32_t*>(m_buffer.data() + data_offset);
				*object_data = static_cast<uint32_t>(object.values.size());

				++object_data;
				for (const auto& value_it : object.values)
				{
					object_data[0] = value_it.first;
					object_data[1] = value_it.second;
					object_data += 2;
				}
			}

			Result internal_open_array() override 
			{
				if (m_current_ref.type != ReferenceType::VALUE)
				{
					return Result::INVALID_CONTAINER;
				}

				// Set the reference type and stack index
				m_current_ref.type = ReferenceType::ARRAY;
				m_current_ref.stack_index = m_array_stack.size();

				// Push reference to stack
				m_reference_stack.push_back(m_current_ref);

				// Add array to stack
				Array& new_array = m_array_stack.emplace_back();

				if (is_reading() == true)
				{
					parse_array_metadata(m_current_ref.value, new_array);
				}

				return Result::SUCCESSFUL;
			}

			void parse_array_metadata(uint32_t data_offset, Array& array)
			{
				array.values.clear();
				const uint32_t* data_ptr = reinterpret_cast<const uint32_t*>(m_buffer.data() + data_offset);

				const uint32_t array_size = *data_ptr;
				++data_ptr;

				if (array_size > 0)
				{
					array.values.resize(array_size);
					memcpy(array.values.data(), data_ptr, array_size * sizeof(uint32_t));
				}
			}

			size_t get_array_size() const override
			{
				const ValueReference& parent_ref = get_parent_ref();
				VADON_ASSERT(parent_ref.type == ReferenceType::ARRAY, "Invalid container!");

				return m_array_stack[parent_ref.stack_index].values.size();
			}

			const ValueReference& get_parent_ref() const
			{
				return (m_reference_stack.empty() == false) ? m_reference_stack.back() : m_root_reference;
			}

			ValueReference& get_parent_ref()
			{
				return const_cast<ValueReference&>(std::as_const(*this).get_parent_ref());
			}

			uint32_t find_string_id(std::string_view key) const
			{
				const std::string key_string(key);
				auto string_id_it = m_string_table.find(key_string);

				if (string_id_it != m_string_table.end())
				{
					return string_id_it->second;
				}

				return 0;
			}

			uint32_t get_string_id(std::string_view key)
			{
				const std::string key_string(key);
				auto string_id_it = m_string_table.find(key_string);

				if (string_id_it != m_string_table.end())
				{
					return string_id_it->second;
				}

				// Only allow new entries if we are writing
				if (is_reading() == true)
				{
					return 0;
				}
				else
				{
					const uint32_t new_string_id = m_string_id_counter++;

					m_string_table.insert(std::make_pair(key_string, new_string_id));
					m_string_lookup.insert(std::make_pair(new_string_id, key_string));

					return new_string_id;
				}
			}

			std::string_view get_string(uint32_t id) const
			{
				auto string_it = m_string_lookup.find(id);
				VADON_ASSERT(string_it != m_string_lookup.end(), "Invalid string ID!");
				return string_it->second;
			}

			void pop_reference_stack()
			{
				m_current_ref = m_reference_stack.back();
				m_reference_stack.pop_back();
			}

			ValueReference m_root_reference;

			ValueReference m_current_ref;
			std::vector<ValueReference> m_reference_stack;

			std::vector<Object> m_object_stack;
			std::vector<Array> m_array_stack;

			std::unordered_map<std::string, uint32_t> m_string_table;
			uint32_t m_string_id_counter = 1;

			std::unordered_map<uint32_t, std::string> m_string_lookup;
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

				m_finalized = true;
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

			Result serialize_color(Vadon::Math::ColorRGBA& color) override
			{
				if (is_reading() == true)
				{
					if (m_value_it->is_string() == false)
					{
						return Result::INVALID_DATA;
					}

					const std::string color_string = m_value_it->get<std::string>();
					if (color_string.front() != '#')
					{
						return Result::INVALID_DATA;
					}
					
					const std::string_view hex_substr = std::string_view(color_string).substr(1);
					if (hex_substr.length() != 8)
					{
						return Result::INVALID_DATA;
					}

					Result result = Result::SUCCESSFUL;
					uint32_t color_uint = 0;
					try
					{
						color_uint = std::strtoul(hex_substr.data(), nullptr, 16);
					}
					catch (std::exception& e)
					{
						Vadon::Core::Logger::log_error(std::format("Serializer error: invalid conversion (\"{}\")", e.what()));
						result = Result::INVALID_DATA;
					}

					// Readable representation has RGBA order, need to reverse to get the memory order used by the engine
					color.value = reverse_uint_bytes(color_uint);
					return result;
				}
				else
				{
					// For readable representation, we want the bytes to be in RGBA order
					uint32_t color_uint = reverse_uint_bytes(color.value);
					std::string color_string = std::format("#{:0>8x}", color_uint);
					return serialize_trivial(color_string);
				}
			}

			Result serialize_uuid(Vadon::Utilities::UUID& value) override
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

	Serializer::Serializer(std::vector<std::byte>& buffer, Mode mode)
		: m_mode(mode)
		, m_buffer(buffer)
	{
	}
}