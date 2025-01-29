#ifndef VADON_UTILITIES_CONTAINER_BOX_HPP
#define VADON_UTILITIES_CONTAINER_BOX_HPP
#include <memory>
namespace Vadon::Utilities
{
    // Taken from https://www.foonathan.net/2022/05/recursive-variant-box/
    template <typename T>
    class Box
    {
    public:
        Box(T&& object) 
            : m_data(new T(std::move(object))) 
        {}

        Box(const T& object)
            : m_data(new T(object))
        {}

        Box(const Box& other)
            : Box(*other.m_data)
        {}

        Box& operator=(const Box& other)
        {
            *m_data = *other.m_data;
            return *this;
        }

        ~Box() = default;

        T& operator*() { return *m_data; }
        const T& operator*() const { return *m_data; }

        T* operator->() { return m_data.get(); }
        const T* operator->() const { return m_data.get(); }
    private:
        std::unique_ptr<T> m_data;
    };

    template<typename T>
    bool operator==(const Box<T>& /*lhs*/, const Box<T>& /*rhs*/)
    {
        // TODO!!!
        return false;
    }

    template<typename T>
    bool operator!=(const Box<T>& /*lhs*/, const Box<T>& /*rhs*/)
    {
        // TODO!!!
        return true;
    }
}
#endif