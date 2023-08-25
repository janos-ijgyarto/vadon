#ifndef VADON_UTILITIES_CONTAINER_UNIQUEVECTOR_HPP
#define VADON_UTILITIES_CONTAINER_UNIQUEVECTOR_HPP
#include <vector>
#include <memory>
#include <functional>
namespace Vadon::Utilities
{
	template<typename T>
	class UniqueVector
	{
	public:
		using Reference = std::reference_wrapper<T>;
		using _Ptr = std::unique_ptr<T>;

		template<typename... Args>
		Reference add(Args&&... args)
		{
			_Ptr& new_obj = m_vector.emplace_back(new T(std::forward<Args>(args)...));
			return *new_obj;
		}
	private:
		std::vector<_Ptr> m_vector;
	};
}
#endif