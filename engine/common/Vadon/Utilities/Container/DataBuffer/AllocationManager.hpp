#ifndef VADON_UTILITIES_CONTAINER_DATABUFFER_ALLOCATIONMANAGER_HPP
#define VADON_UTILITIES_CONTAINER_DATABUFFER_ALLOCATIONMANAGER_HPP
#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <vector>
namespace Vadon::Utilities
{
	using DataAllocationID = int32_t;

	struct DataAllocation
	{
		static constexpr DataAllocationID InvalidID = DataAllocationID(-1);

		DataAllocationID id = InvalidID;
		DataRange range;

		bool is_valid() const { return (id != InvalidID) && range.is_valid(); }
	};

	// TODO: implement a variant which can "compact" the allocations (i.e sparse-dense set approach)
	class DataAllocationManager
	{
	public:
		DataAllocationManager();

		void reset();

		DataAllocation allocate(int32_t count);
		void release(const DataAllocation& allocation);

		int32_t get_size() const;
	protected:
		using AllocationVector = std::vector<DataAllocation>;
		AllocationVector m_allocations;

		AllocationVector::iterator find_slot(int32_t count);
		DataAllocation allocate_internal(AllocationVector::iterator slot, int32_t count);

		DataAllocationID m_allocation_counter;
		DataAllocationID m_min_allocation_id;
	};
}
#endif