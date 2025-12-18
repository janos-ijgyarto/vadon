#ifndef VADON_ECS_COMPONENT_COMPONENT_HPP
#define VADON_ECS_COMPONENT_COMPONENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::ECS
{
	using ComponentID = Vadon::Utilities::TypeID;
	using ComponentIDList = std::vector<ComponentID>;

	class ComponentManager;
	class ComponentPoolInterface;
	class ComponentHandle
	{
	public:
		VADONCOMMON_API const void* get_raw() const;
		void* get_raw() { return const_cast<void*>(std::as_const(*this).get_raw()); }

		VADONCOMMON_API bool is_valid() const;
	protected:
		ComponentHandle(ComponentPoolInterface* pool, EntityHandle owner)
			: m_pool(pool), m_owner(owner)
		{
		}

		ComponentPoolInterface* m_pool;
		EntityHandle m_owner;
		
		friend ComponentManager;
	};

	template<typename T>
	class TypedComponentHandle : public ComponentHandle
	{
	public:
		const T& operator *() const;
		T& operator *() { return const_cast<T&>(std::as_const(*this).operator*()); }

		const T* operator->() const;
		T* operator->() { return const_cast<T*>(std::as_const(*this).operator->()); }
	private:
		TypedComponentHandle(ComponentPoolInterface* pool, EntityHandle owner)
			: ComponentHandle(pool, owner)
		{
		}

		friend ComponentManager;
	};

}
#endif