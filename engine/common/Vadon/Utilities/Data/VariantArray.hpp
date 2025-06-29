#ifndef VADON_UTILITIES_DATA_VARIANTARRAY_HPP
#define VADON_UTILITIES_DATA_VARIANTARRAY_HPP
#include <Vadon/Utilities/Data/VariantBase.hpp>
#include <Vadon/Utilities/TypeInfo/ErasedDataType.hpp>
namespace Vadon::Utilities
{
	using VariantVector = std::vector<Variant>;
	struct VariantArray
	{
		VariantVector data;
		ErasedDataTypeID data_type;

		bool operator==(const VariantArray& other) const
		{
			if (this->data_type != other.data_type)
			{
				return false;
			}

			if (this->data.size() == other.data.size())
			{
				for (size_t i = 0; i < this->data.size(); ++i)
				{
					if (this->data[i] != other.data[i])
					{
						return false;
					}
				}

				return true;
			}

			return false;
		}

		bool operator!=(const VariantArray& other) const
		{
			if (this->data.size() != other.data.size())
			{
				return true;
			}

			for (size_t i = 0; i < this->data.size(); ++i)
			{
				if (this->data[i] != other.data[i])
				{
					return true;
				}
			}

			return false;
		}
	};

	template<typename T>
	using as_std_vector = std::vector<typename T::value_type, typename T::allocator_type>;

	template<typename T>
	concept is_std_vector = std::same_as<T, as_std_vector<T>>;

	template<is_std_vector T>
	Variant to_variant(const T& vector)
	{
		VariantArray array;
		array.data_type = get_erased_data_type_id<typename T::value_type>();
		for (const T::value_type& value : vector)
		{
			array.data.push_back(to_variant(value));
		}
		return Variant(Box(array));
	}

	template<is_std_vector T>
	T from_variant(const Variant& variant)
	{
		T vector;
		const VariantArray& variant_array = *std::get<Box<VariantArray>>(variant);
		for (const Variant& value : variant_array.data)
		{
			vector.push_back(from_variant<typename T::value_type>(value));
		}

		return vector;
	}
}
#endif