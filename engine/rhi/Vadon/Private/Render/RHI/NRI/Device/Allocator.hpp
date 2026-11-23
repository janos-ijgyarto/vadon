#ifndef VADON_PRIVATE_RENDER_RHI_NRI_DEVICE_ALLOCATOR_HPP
#define VADON_PRIVATE_RENDER_RHI_NRI_DEVICE_ALLOCATOR_HPP
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <nri/Extensions/NRIDeviceCreation.h>
#include <atomic>
namespace Vadon::Private::Render::RHI::NRI
{
    // NOTE: implementation taken from NRIFramework
    struct DebugAllocatorHeader
    {
        size_t size;
        uint32_t alignment;
        uint32_t offset;
    };

    struct DebugAllocator
    {
        std::atomic_uint64_t allocation_num = 0;
        std::atomic_size_t allocated_size = 0;

        template <typename T, typename A>
        static constexpr T get_aligned_size(T x, A alignment)
        {
            return (T)((size_t(x) + (size_t)alignment - 1) & ~((size_t)alignment - 1));
        }

        inline static DebugAllocatorHeader* get_header(void* memory)
        {
            return (DebugAllocatorHeader*)memory - 1;
        }

        static void* aligned_malloc(void* userArg, size_t size, size_t alignment)
        {
            VADON_ASSERT(alignment != 0, "Invalid alignment!");

            size_t aligned_header_size = DebugAllocator::get_aligned_size(sizeof(DebugAllocatorHeader), alignment);
            size_t allocation_size = size + alignment - 1 + aligned_header_size;

            uint8_t* memory = (uint8_t*)::malloc(allocation_size);
            if (memory == nullptr)
            {
                return nullptr;
            }

            uint8_t* aligned_memory = DebugAllocator::get_aligned_size(memory, alignment) + aligned_header_size;

            DebugAllocatorHeader* header = get_header(aligned_memory);
            *header = {};
            header->size = allocation_size;
            header->alignment = (uint32_t)alignment;
            header->offset = (uint32_t)(aligned_memory - memory);

            DebugAllocator* allocator = (DebugAllocator*)userArg;
            allocator->allocated_size.fetch_add(allocation_size, std::memory_order_relaxed);
            allocator->allocation_num.fetch_add(1, std::memory_order_relaxed);

            return aligned_memory;
        }

        static void* aligned_realloc(void* user_arg, void* memory, size_t size, size_t alignment) 
        {
            VADON_ASSERT(alignment != 0, "Invalid alignment");

            if (!memory)
            {
                return aligned_malloc(user_arg, size, alignment);
            }

            const DebugAllocatorHeader prev_header = *get_header(memory);
            VADON_ASSERT(prev_header.alignment == alignment, "Invalid header!");

            const size_t aligned_header_size = get_aligned_size(sizeof(DebugAllocatorHeader), alignment);
            const size_t allocation_size = size + alignment - 1 + aligned_header_size;

            uint8_t* prev_memory_begin = (uint8_t*)memory - prev_header.offset;

            uint8_t* new_memory = (uint8_t*)::realloc(prev_memory_begin, allocation_size);
            if (new_memory == nullptr)
            {
                return nullptr;
            }

            uint8_t* aligned_memory = get_aligned_size(new_memory, alignment) + aligned_header_size;

            DebugAllocator* allocator = (DebugAllocator*)user_arg;
            allocator->allocated_size.fetch_add(allocation_size - prev_header.size, std::memory_order_relaxed);

            DebugAllocatorHeader* newHeader = get_header(aligned_memory);
            *newHeader = {};
            newHeader->size = allocation_size;
            newHeader->alignment = (uint32_t)alignment;
            newHeader->offset = (uint32_t)(aligned_memory - new_memory);

            return aligned_memory;
        }

        static void aligned_free(void* user_arg, void* memory) 
        {
            if (memory == nullptr)
            {
                return;
            }

            const DebugAllocatorHeader* header = get_header(memory);

            DebugAllocator* allocator = (DebugAllocator*)user_arg;
            [[maybe_unused]] size_t allocated_size = allocator->allocated_size.fetch_sub(header->size, std::memory_order_relaxed);
            [[maybe_unused]] size_t allocation_num = allocator->allocation_num.fetch_sub(1, std::memory_order_relaxed);

            VADON_ASSERT(allocated_size >= header->size, "Invalid allocator state!");
            VADON_ASSERT(allocation_num != 0, "Invalid allocator state!");

            ::free((uint8_t*)memory - header->offset);
        }

        static void create_debug_allocator(nri::AllocationCallbacks& allocation_callbacks)
        {
            allocation_callbacks = {};
            allocation_callbacks.userArg = new DebugAllocator();
            allocation_callbacks.Allocate = &DebugAllocator::aligned_malloc;
            allocation_callbacks.Reallocate = &DebugAllocator::aligned_realloc;
            allocation_callbacks.Free = &DebugAllocator::aligned_free;
        }

        static void destroy_debug_allocator(nri::AllocationCallbacks& allocation_callbacks)
        {
            DebugAllocator* debug_allocator = (DebugAllocator*)allocation_callbacks.userArg;

            size_t size = debug_allocator->allocated_size.load(std::memory_order_relaxed);
            size_t num = debug_allocator->allocation_num.load(std::memory_order_relaxed);
            if (size || num)
            {
                Vadon::Core::Logger::log_warning(std::format("Vadon::Private::Render::RHI::NRI::DebugAllocator::destroy_debug_allocator(): {} bytes left in {} allocations!\n", (int32_t)size, (int32_t)num));
            }

            delete debug_allocator;
        }
    };
}
#endif