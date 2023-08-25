#ifndef VADON_UTILITIES_CONTAINER_CONCURRENT_TRIPLEBUFFER_HPP
#define VADON_UTILITIES_CONTAINER_CONCURRENT_TRIPLEBUFFER_HPP
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace Vadon::Utilities
{
    // Taken from: https://codereview.stackexchange.com/questions/259929/lock-free-triple-buffer
    // FIXME: might be worth making reader and writer interfaces into their own objects?
    template<typename T>
    class TripleBuffer
    {
    public:
        using BufferArray = std::array<T, 3>;

        TripleBuffer(const BufferArray& init = BufferArray())
            : m_buffer(init)
            , m_read_buffer(&m_buffer[0])
            , m_write_buffer(&m_buffer[1])
            , m_available(&m_buffer[2])
            , m_next_read_buffer(nullptr)
        {

        }

        // Writer interface

        // Get buffer for producer to write to (non-blocking)
        // FIXME: could make reference, and internally swap pointer?
        T* get_write_buffer()
        {
            return m_write_buffer;
        }

        // Producer signals that it's finished writing to the buffer
        // FIXME: should use RAII object!
        void set_write_complete()
        {
            // Swap the write buffer and the available buffer
            auto* finished_write_buffer = m_write_buffer;
            m_write_buffer = m_available.exchange(m_write_buffer);

            // Mark the finished buffer as the one to read from (this will tell the consumer that it's ready to go)
            m_next_read_buffer.store(finished_write_buffer, std::memory_order_release);

            // Notify the reader (if it's waiting)
            m_read_queue.notify_one();
        }

        // Reader interface

        // Consumer gets read buffer, keeping ownership until next call of this function (other two will get swapped between write and available)
        T* get_read_buffer(std::chrono::milliseconds timeout = std::chrono::milliseconds::max())
        {
            auto const timeout_time = std::chrono::steady_clock::now() + timeout;

            // Get the written buffer, waiting if necessary
            // (setting to nullptr means the consumer has attempted to consume the output of the producer)
            auto* next_read_buffer = m_next_read_buffer.exchange(nullptr);

            while (next_read_buffer != m_read_buffer)
            {
                // Next read has changed, check whether the producer has placed something in available
                // Also swaps read and available, to expose read buffers to writing
                m_read_buffer = m_available.exchange(m_read_buffer);
                if (next_read_buffer == m_read_buffer)
                {
                    // Available and next read match, good to go
                    return m_read_buffer;
                }

                // Still need to wait on the producer (might still be writing, or hasn't swapped available yet)
                next_read_buffer = nullptr;

                std::unique_lock lock{ m_read_queue_mutex };

                auto test = [this, &next_read_buffer]
                {
                    // While we wait, keep checking next read in case the producer has set it (will return true once we get something non-nullptr)
                    next_read_buffer = m_next_read_buffer.exchange(nullptr);
                    return next_read_buffer;
                };

                // Wait until next read changes, or we time out)
                if (!m_read_queue.wait_until(lock, timeout_time, test))
                {
                    // Timed out, consumer will have to try again later
                    return nullptr;
                }
            }

            // "Next read" still the same as current (if nullptr, then we still need to wait on the producer)
            return m_read_buffer;
        }
    private:
        BufferArray m_buffer;

        T* m_read_buffer; // Available only to consumer
        T* m_write_buffer; // Available only to producer
        std::atomic<T*> m_available; // Swapped between consumer and producer

        std::atomic<T*> m_next_read_buffer; // Last fully-written buffer, waiting for consumer

        // Mutex and CV in case consumer catches up to producer
        std::mutex m_read_queue_mutex;
        std::condition_variable m_read_queue;
    };
}
#endif