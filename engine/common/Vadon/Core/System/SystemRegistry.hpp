#ifndef VADON_CORE_SYSTEM_SYSTEMREGISTRY_HPP
#define VADON_CORE_SYSTEM_SYSTEMREGISTRY_HPP
#include <Vadon/Utilities/System/Singleton/Registry.hpp>
#include <vector>
namespace Vadon::Core
{
	class SystemRegistry
	{
	public:
		template<typename Sys> const Sys& get_system() const
		{
			return m_singleton_registry.get<Sys>();
		}

		template<typename Sys> Sys& get_system()
		{
			return m_singleton_registry.get<Sys>();
		}
	private:
		Utilities::SingletonRegistry m_singleton_registry;
		friend class SystemBase;
	};
}
#endif