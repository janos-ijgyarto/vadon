#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>

#include <Vadon/Utilities/TypeInfo/TypeList.hpp>
#include <Vadon/Utilities/Data/Visitor.hpp>

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
			BinarySerializer(std::vector<std::byte>& buffer, bool read)
				: Serializer(buffer, read)
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

			bool serialize(std::string_view /*key*/, int& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(std::string_view /*key*/, float& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(std::string_view /*key*/, std::string& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(std::string_view /*key*/, Variant& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(size_t /*index*/, int& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(size_t /*index*/, float& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(size_t /*index*/, std::string& /*value*/) override
			{
				// TODO
				return false;
			}

			bool serialize(size_t /*index*/, Variant& /*value*/) override
			{
				// TODO
				return false;
			}

			bool open_array(std::string_view /*key*/) override
			{
				// TODO
				return false;
			}

			bool open_array(size_t /*index*/) override
			{
				// TODO
				return false;
			}

			bool close_array() override
			{
				// TODO
				return false;
			}

			bool open_object(std::string_view /*key*/) override
			{
				// TODO
				return false;
			}

			bool open_object(size_t /*index*/) override
			{
				// TODO
				return false;
			}

			bool close_object() override
			{
				// TODO
				return false;
			}
		protected:
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

			JSONSerializer(std::vector<std::byte>& buffer, bool read)
				: Serializer(buffer, read)
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
						m_last_error = std::format("Error parsing JSON data: {}\nException ID: {}\nError byte position: {}\n", exception.what(), exception.id, exception.byte);
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
				if (is_reading() == false)
				{
					// Dump JSON to buffer
					// FIXME: is this the right way to do it?
					const std::string json_dump = m_json_root.dump(4);
					m_buffer.resize(json_dump.size());
					memcpy(m_buffer.data(), json_dump.data(), json_dump.size());
				}
				else
				{
					// TODO: clear memory?
				}

				return true;
			}

			bool serialize(std::string_view key, int& value) override
			{
				return serialize_trivial(key, value);
			}

			bool serialize(std::string_view key, float& value)override
			{
				return serialize_trivial(key, value);
			}

			bool serialize(std::string_view key, std::string& value) override
			{
				return serialize_trivial(key, value);
			}

			bool serialize(std::string_view key, Variant& value) override
			{
				return serialize_variant(key, value);
			}

			bool serialize(size_t index, int& value) override
			{
				return serialize_trivial(index, value);
			}

			bool serialize(size_t index, float& value) override
			{
				return serialize_trivial(index, value);
			}

			bool serialize(size_t index, std::string& value) override
			{
				return serialize_trivial(index, value);
			}

			bool serialize(size_t index, Variant& value) override
			{
				return serialize_variant(index, value);
			}

			bool open_array(std::string_view key) override
			{
				// Can only open array with key within an object
				JSON& current_object = get_current_object();
				if (current_object.is_object() == true)
				{
					Iterator array_it = current_object.find(key);
					if (is_reading() == true)
					{
						if ((array_it != current_object.end()) && (array_it->is_array() == true))
						{
							push_iterator_stack(array_it);
							return true;
						}
					}
					else
					{
						// Make sure key is unique
						if (array_it == current_object.end())
						{
							array_it = current_object.emplace(key, JSON::array()).first;
							push_iterator_stack(array_it);
							return true;
						}
					}
				}

				return false;
			}

			bool open_array(size_t index) override
			{
				// Can only open array with index within an array
				JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					if (is_reading() == true)
					{
						// Can read anywhere in the array
						if (index < current_object.size())
						{
							Iterator array_it = current_object.begin() + index;
							if (array_it->is_array() == true)
							{
								push_iterator_stack(array_it);
								return true;
							}
						}
					}
					else
					{
						// Must always be adding to the end of the array
						if (index == current_object.size())
						{
							current_object.push_back(JSON::array());
							push_iterator_stack(current_object.begin() + index);
							return true;
						}
					}
				}

				return false;
			}

			bool close_array() override
			{
				// Only allow if we were in an array to begin with
				JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					return pop_iterator_stack();
				}
				return false;
			}

			bool open_object(std::string_view key) override
			{
				// Can only open object with key within an object
				JSON& current_object = get_current_object();
				if (current_object.is_object() == true)
				{
					Iterator object_it = current_object.find(key);
					if (is_reading() == true)
					{
						if ((object_it != current_object.end()) && (object_it->is_object() == true))
						{
							push_iterator_stack(object_it);
							return true;
						}
					}
					else
					{
						// Make sure key is unique
						if (object_it == current_object.end())
						{
							object_it = current_object.emplace(key, JSON::object()).first;
							push_iterator_stack(object_it);
							return true;
						}
					}
				}
				return false;
			}

			bool open_object(size_t index) override
			{
				// Can only open object with index within an array
				JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					if (is_reading() == true)
					{
						// Can read anywhere in the array
						if (index < current_object.size())
						{
							Iterator object_it = current_object.begin() + index;
							if (object_it->is_object() == true)
							{
								push_iterator_stack(object_it);
								return true;
							}
						}
					}
					else
					{
						// Must always be adding to the end of the array
						if (index == current_object.size())
						{
							current_object.push_back(JSON::object());
							push_iterator_stack(current_object.begin() + index);
							return true;
						}
					}
				}
				return false;
			}

			bool close_object() override
			{
				// Only allow if we were in an object to begin with
				JSON& current_object = get_current_object();
				if (current_object.is_object() == true)
				{
					return pop_iterator_stack();
				}
				return false;
			}
		protected:
			template<typename T>
			bool serialize_trivial(std::string_view key, T& value)
			{
				// Can only serialize with key in an object
				JSON& current_object = get_current_object();
				if (current_object.is_object() == true)
				{
					Iterator value_it = current_object.find(key);
					if (is_reading() == true)
					{
						// FIXME: ensure that type matches?
						if ((value_it != current_object.end()) && value_it->is_primitive())
						{
							value = value_it.value().get<T>();
							return true;
						}
					}
					else
					{
						if (value_it == current_object.end())
						{
							current_object[key] = value;
							return true;
						}
					}
				}
				return false;
			}

			bool serialize_variant(std::string_view key, Variant& value)
			{
				// Can only serialize with key in an object
				JSON& current_object = get_current_object();
				if (current_object.is_object() == true)
				{
					Iterator value_it = current_object.find(key);
					if (is_reading() == true)
					{
						if (value_it != current_object.end())
						{
							return read_json_to_variant(value_it.value(), value);
						}
					}
					else
					{
						if (value_it == current_object.end())
						{
							return write_variant_to_json(key, value);
						}
					}
				}
				return false;
			}

			template<typename T>
			bool serialize_trivial(size_t index, T& value)
			{
				// Can only serialize with index in an array
				JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					if (is_reading() == true)
					{
						// Can read anywhere in the array
						if (index < current_object.size())
						{
							JSON& value_it = current_object.at(index);
							// FIXME: ensure that type matches?
							if (value_it.is_primitive())
							{
								value = value_it.get<T>();
								return true;
							}
						}
					}
					else
					{
						// Must add to end of array
						if (index == current_object.size())
						{
							current_object.push_back(value);
							return true;
						}
					}
				}
				return false;
			}

			bool serialize_variant(size_t index, Variant& value)
			{
				// Can only serialize with index in an array
				JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					if (is_reading() == true)
					{
						// Can read anywhere in the array
						if (index < current_object.size())
						{
							JSON& value_it = current_object.at(index);
							return read_json_to_variant(value_it, value);
						}
					}
					else
					{
						// Must add to end of array
						if (index == current_object.size())
						{
							JSON& json_value = current_object.emplace_back();							
							return write_variant_to_json(json_value, value);
						}
					}
				}
				return false;
			}

			bool read_json_to_variant(const JSON& json_value, Variant& variant_value)
			{
				// FIXME: better way to do this?
				// FIXME2: support double separately?
				if (json_value.is_object() == true)
				{
					// TODO!!!
				}
				else if (json_value.is_array() == true)
				{
					// FIXME: proper lookup system!
					const std::string& type_string = json_value[0];
					if (type_string == "Vector2")
					{
						variant_value = Vadon::Utilities::Vector2(json_value[1].get<float>(), json_value[2].get<float>());
						return true;
					}
					else if (type_string == "Vector3")
					{
						variant_value = Vadon::Utilities::Vector3(json_value[1].get<float>(), json_value[2].get<float>(), json_value[3].get<float>());
						return true;
					}
					else if (type_string == "Vector4")
					{
						variant_value = Vadon::Utilities::Vector4(json_value[1].get<float>(), json_value[2].get<float>(), json_value[3].get<float>(), json_value[4].get<float>());
						return true;
					}
				}
				else
				{
					switch (json_value.type())
					{
					case Vadon::Utilities::JSONValueType::number_integer:
						variant_value = json_value.get<int>();
						return true;
					case Vadon::Utilities::JSONValueType::number_float:
						variant_value = json_value.get<float>();
						return true;
					case Vadon::Utilities::JSONValueType::boolean:
						variant_value = json_value.get<bool>();
						return true;
					case Vadon::Utilities::JSONValueType::string:
						variant_value = json_value.get<std::string>();
						return true;
						// TODO: default?
					}
				}
				return false;
			}

			bool write_variant_to_json(std::string_view key, const Variant& value)
			{
				// FIXME: check whether we can write the value?
				auto variant_visitor = Vadon::Utilities::VisitorOverloadList{
					[&](auto value)
					{
						if constexpr (IsTrivialJSONWrite<decltype(value)>)
						{
							get_current_object()[key] = value;
						}
						if constexpr (std::is_same_v<decltype(value), Vadon::Utilities::Vector2> == true)
						{
							// FIXME: have a lookup for the keys!
							get_current_object()[key] = Vadon::Utilities::JSON::array({ "Vector2", value.x, value.y });
						}
						if constexpr (std::is_same_v<decltype(value), Vadon::Utilities::Vector3> == true)
						{
							// FIXME: have a lookup for the keys!
							get_current_object()[key] = Vadon::Utilities::JSON::array({ "Vector3", value.x, value.y, value.z });
						}
						if constexpr (std::is_same_v<decltype(value), Vadon::Utilities::Vector4> == true)
						{
							// FIXME: have a lookup for the keys!
							get_current_object()[key] = Vadon::Utilities::JSON::array({ "Vector4", value.x, value.y, value.z, value.w });
						}
						// TODO: other overloads?
					}
				};

				std::visit(variant_visitor, value);
				return true;
			}

			size_t get_array_size() const override
			{
				const JSON& current_object = get_current_object();
				if (current_object.is_array() == true)
				{
					return current_object.size();
				}
				return 0;
			}

			const JSON& get_current_object() const
			{
				return m_iterator_stack.empty() ? m_json_root : *m_iterator_stack.back();
			}

			JSON& get_current_object()
			{
				return const_cast<JSON&>(std::as_const(*this).get_current_object());
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
		};
	}

	Serializer::Instance Serializer::create_serializer(std::vector<std::byte>& buffer, Type type, bool read)
	{
		switch (type)
		{
		case Type::BINARY:
			return Serializer::Instance(new BinarySerializer(buffer, read));
		case Type::JSON:
			return Serializer::Instance(new JSONSerializer(buffer, read));
		}
		return nullptr;
	}

	Serializer::Serializer(std::vector<std::byte>& buffer, bool read)
		: m_read(read)
		, m_buffer(buffer)
	{
	}
}