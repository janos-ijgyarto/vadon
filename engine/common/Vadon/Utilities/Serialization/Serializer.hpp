#ifndef VADON_UTILITIES_SERIALIZATION_SERIALIZER_HPP
#define VADON_UTILITIES_SERIALIZATION_SERIALIZER_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/Math/Color.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
#include <vector>
#include <memory>
#include <string>
#include <span>
namespace Vadon::Utilities
{
	struct UUID;

	class Serializer
	{
	public:
		enum class Type : uint8_t
		{
			BINARY,
			JSON
		};

		enum class Mode : uint8_t
		{
			READ,
			WRITE
		};

		enum class Result : uint8_t
		{
			NOT_IMPLEMENTED,
			INVALID_KEY,
			INVALID_CONTAINER,
			INVALID_DATA,
			SUCCESSFUL
		};

		using Instance = std::unique_ptr<Serializer>;

		VADONCOMMON_API static Instance create_serializer(std::vector<std::byte>& buffer, Type type, Mode mode);

		virtual ~Serializer() {}

		bool is_reading() const { return m_mode == Mode::READ; }

		virtual bool initialize() = 0;
		virtual bool finalize() = 0;

		// TODO: have some way to get the type of the data stored in the serializer?
		// Or do we expect client code to know what to deserialize?

		// Object members
		template<typename T>
		Result serialize(std::string_view key, T& value)
		{
			const Result key_result = set_value_reference(key);
			if(key_result == Result::SUCCESSFUL)
			{
				return internal_serialize_value<T>(value);
			}

			return key_result;
		}

		// Array elements
		template<typename T>
		Result serialize(size_t index, T& value)
		{
			const Result index_result = set_value_reference(index);
			if (index_result == Result::SUCCESSFUL)
			{
				return internal_serialize_value<T>(value);
			}

			return index_result;
		}

		VADONCOMMON_API Result open_array(std::string_view key);
		VADONCOMMON_API Result open_array(size_t index);

		virtual size_t get_array_size() const = 0;
		virtual Result close_array() = 0;

		virtual bool has_key(std::string_view key) const = 0;
		// TODO: get list of keys from object?

		VADONCOMMON_API Result open_object(std::string_view key);
		VADONCOMMON_API Result open_object(size_t index);
		virtual Result close_object() = 0;
	protected:
		Serializer(std::vector<std::byte>& buffer, Mode mode);

		virtual Result set_value_reference(std::string_view key) = 0;
		virtual Result set_value_reference(size_t index) = 0;

		virtual Result internal_open_object() = 0;
		virtual Result internal_open_array() = 0;

		template<typename T>
		Result internal_serialize_value(T& value)
		{
			static_assert(false, "Serialization not implemented for type!");
			return Result::NOT_IMPLEMENTED;
		}

		template<typename T>
		Result serialize_array_values(std::span<T, std::dynamic_extent> values)
		{
			internal_open_array();
			for (size_t value_index = 0; value_index < values.size(); ++value_index)
			{
				// Boilerplate to deal with T being pointers or contiguous memory
				T* value_ptr = nullptr;
				if constexpr (std::is_pointer_v<T>)
				{
					value_ptr = values[value_index];
				}
				else
				{
					value_ptr = &values[value_index];
				}
				const Result current_result = serialize(value_index, *value_ptr);
				if (current_result != Result::SUCCESSFUL)
				{
					return current_result;
				}
			}
			close_array();

			return Result::SUCCESSFUL;
		}

		template<> Result internal_serialize_value(int& value) { return serialize_int(value); }
		template<> Result internal_serialize_value(float& value) { return serialize_float(value); }
		template<> Result internal_serialize_value(bool& value) { return serialize_bool(value); }
		template<> Result internal_serialize_value(std::string& value) { return serialize_string(value); }
		template<> Result internal_serialize_value(Vadon::Utilities::UUID& value) { return serialize_uuid(value); }

		// TODO: can make a template wrapper for all vectors with size param (just point to first value)
		template<> Result internal_serialize_value(Vadon::Utilities::Vector2& value) { return serialize_array_values(std::span(&value.x, 2)); }
		template<> Result internal_serialize_value(Vadon::Utilities::Vector2i& value) { return serialize_array_values(std::span(&value.x, 2)); }

		template<> Result internal_serialize_value(Vadon::Utilities::Vector3& value) { return serialize_array_values(std::span(&value.x, 3)); }

		template<> Result internal_serialize_value(Vadon::Utilities::ColorRGBA& value) { return serialize_color(value); }

		virtual Result serialize_int(int& value) = 0;
		virtual Result serialize_float(float& value) = 0;
		virtual Result serialize_bool(bool& value) = 0;
		virtual Result serialize_string(std::string& value) = 0;
		virtual Result serialize_color(ColorRGBA& color) = 0;

		VADONCOMMON_API Result serialize_uuid(Vadon::Utilities::UUID& value);

		std::vector<std::byte>& m_buffer;
		Mode m_mode;
	};
}
#endif