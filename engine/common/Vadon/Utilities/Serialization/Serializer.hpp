#ifndef VADON_UTILITIES_SERIALIZATION_SERIALIZER_HPP
#define VADON_UTILITIES_SERIALIZATION_SERIALIZER_HPP
#include <vector>
#include <memory>
#include <string>
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Utilities
{
	class Serializer
	{
	public:
		enum class Type
		{
			BINARY,
			JSON
		};

		using Instance = std::unique_ptr<Serializer>;

		VADONCOMMON_API static Instance create_serializer(std::vector<std::byte>& buffer, Type type, bool read = true);

		virtual ~Serializer() {}

		bool is_reading() const { return m_read; }

		// FIXME: implement a more effective interface (possibly via std::optional or std::expected)
		// which can both replace the bool return values AND decouple logging
		// NOTE: may need to attach a logger to keep the API less verbose?
		// Best approach will be to make the logger global
		const std::string& get_last_error() const { return m_last_error; }

		virtual bool initialize() = 0;
		virtual bool finalize() = 0;

		// Object members
		virtual bool serialize(std::string_view key, int& value) = 0;
		virtual bool serialize(std::string_view key, float& value) = 0;
		virtual bool serialize(std::string_view key, std::string& value) = 0;
		virtual bool serialize(std::string_view key, Variant& value) = 0;

		// Array elements
		virtual bool serialize(size_t index, int& value) = 0;
		virtual bool serialize(size_t index, float& value) = 0;
		virtual bool serialize(size_t index, std::string& value) = 0;
		virtual bool serialize(size_t index, Variant& value) = 0;

		// TODO: this is the "do whatever you want" API, vector will be when you enforce the same type
		virtual bool open_array(std::string_view key) = 0;
		virtual bool open_array(size_t index) = 0;
		virtual size_t get_array_size() const = 0;
		virtual bool close_array() = 0;

		virtual bool open_object(std::string_view key) = 0;
		virtual bool open_object(size_t index) = 0;
		virtual bool close_object() = 0;

		template<typename T>
		bool serialize_vector(std::string_view key, std::vector<T>& vector)
		{
			if (is_reading() == true)
			{
				vector.clear();
			}

			if (open_array(key) == true)
			{
				if (is_reading() == true)
				{
					const size_t array_size = get_array_size();
					T value;
					for (size_t current_index = 0; current_index < array_size; ++array_size)
					{
						serialize("", value);
						vector.push_back(value);
					}
				}
				else
				{
					for (const auto& value : vector)
					{
						serialize("", value);
					}
				}
				close_array();
			}

			return false;
		}
	protected:
		Serializer(std::vector<std::byte>& buffer, bool read);

		std::vector<std::byte>& m_buffer;
		bool m_read;
		std::string m_last_error;
	};
}
#endif