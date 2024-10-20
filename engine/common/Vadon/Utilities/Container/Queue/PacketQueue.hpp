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
		struct Header
		{
			uint32_t packet_id;
			uint32_t data_size;
			// TODO: anything else?
		};

		class Iterator
		{		
		public:
			bool is_valid() const { return m_data_it != m_data_end; }

			void advance() { m_data_it += get_header().data_size + sizeof(Header); }

			const Header& get_header() const { return *reinterpret_cast<const Header*>(std::to_address(m_data_it)); }
			const std::byte* get_packet_data() const
			{
				// TODO: assert if invalid!
				return std::to_address(m_data_it) + sizeof(Header);
			}

			template<typename T>
			const T* get_packet() const
			{
				// FIXME: could this be static_cast?
				return reinterpret_cast<const T*>(get_packet_data());
			}
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