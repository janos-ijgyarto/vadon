#ifndef VADON_UTILITIES_CONTAINER_QUEUE_PACKETQUEUE_HPP
#define VADON_UTILITIES_CONTAINER_QUEUE_PACKETQUEUE_HPP
#include <Vadon/Common.hpp>
#include <vector>
namespace Vadon::Utilities
{
	// TODO: could be an extension of serializer API to reuse the internals?
	class PacketQueue
	{
	private:
		using Buffer = std::vector<std::byte>;
		using BufferIterator = Buffer::const_iterator;
	public:
		class Iterator
		{		
		public:
			bool is_valid() const { return m_data_it != m_data_end; }

			// FIXME: implement template function that advances with size of type
			VADONCOMMON_API void advance(size_t block_size);

			VADONCOMMON_API uint32_t get_header_id() const;
			VADONCOMMON_API const std::byte* get_packet_data() const;
			// TODO: add utility function to cast the data pointer?
		private:
			Iterator(const PacketQueue& queue, size_t start_offset) : m_data_it(queue.m_data.cbegin() + start_offset), m_data_end(queue.m_data.cend()) {}
			BufferIterator m_data_it;
			BufferIterator m_data_end;

			friend PacketQueue;
		};

		bool is_empty() const { return m_data.empty(); }
		size_t get_size() const { return m_data.size(); }

		template<typename T>
		T* allocate_object(uint32_t id)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			std::byte* data_ptr = allocate_raw_data(id, sizeof(T));
			return new(data_ptr)T;
		}

		VADONCOMMON_API std::byte* allocate_raw_data(uint32_t id, size_t size);

		template<typename T>
		void write_object(uint32_t id, const T& object)
		{
			T* obj_ptr = allocate_object<T>(id);
			*obj_ptr = object;
		}

		void append_queue(const PacketQueue& other) { m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end()); }

		Iterator get_iterator(size_t start_offset = 0) const { return Iterator(*this, start_offset); }

		void clear() { m_data.clear(); }
	private:
		std::vector<std::byte> m_data;
	};
}
#endif