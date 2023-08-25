#ifndef VADON_UTILITIES_DATA_DATAUTILITIES_HPP
#define VADON_UTILITIES_DATA_DATAUTILITIES_HPP
namespace Vadon::Utilities
{
	struct DataRange
	{
		static constexpr int32_t InvalidOffset = int32_t(-1);

		int32_t offset = InvalidOffset;
		int32_t count = 0;

		bool is_valid() const { return (offset != InvalidOffset); }
	};
}
#endif