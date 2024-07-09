#ifndef VADONEDITOR_CORE_SYSTEM_REGISTRY_HPP
#define VADONEDITOR_CORE_SYSTEM_REGISTRY_HPP
#include <Vadon/Utilities/System/Singleton/Registry.hpp>
#include <vector>
namespace VadonEditor::Core
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
			return const_cast<Sys&>(std::as_const(*this).get_system<Sys>());
		}
	private:
		Vadon::Utilities::SingletonRegistry m_singleton_registry;
		friend class SystemBase;
	};
}
#endif