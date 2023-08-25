#ifndef VADON_UTILITIES_CONTAINER_CONCURRENT_CONCURRENTQUEUE_HPP
#define VADON_UTILITIES_CONTAINER_CONCURRENT_CONCURRENTQUEUE_HPP
#include <concurrentqueue.h>
namespace Vadon::Utilities
{
	template<typename T>
	using ConcurrentQueue = moodycamel::ConcurrentQueue<T>;
}
#endif