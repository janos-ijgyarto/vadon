#ifndef VADON_CORE_SYSTEM_SYSTEMREGISTRY_HPP
#define VADON_CORE_SYSTEM_SYSTEMREGISTRY_HPP
#include <Vadon/Common.hpp>
#include <vector>
namespace Vadon::Core
{
	class SystemBase;

	class SystemRegistry
	{
	public:
		template<typename Sys> const Sys& get_system() const
		{
			return *static_cast<const Sys*>(get_system_internal(Sys::_Module::get_type_id(), Sys::get_type_id()));
		}

		template<typename Sys> Sys& get_system()
		{
			return const_cast<Sys&>(const_cast<const SystemRegistry*>(this)->get_system<Sys>());
		}
	protected:
		VADONCOMMON_API void register_system(SystemBase* system, size_t module_index, size_t system_index);
	private:
		VADONCOMMON_API const SystemBase* get_system_internal(size_t module_index, size_t system_index) const;

		using OffsetList = std::vector<size_t>;
		std::vector<OffsetList> m_module_system_offsets;
		std::vector<Vadon::Core::SystemBase*> m_system_list;

		friend class SystemBase;
	};
}
#endif