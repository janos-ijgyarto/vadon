#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/System/SystemRegistry.hpp>
namespace Vadon::Core
{
    const Vadon::Core::SystemBase* SystemRegistry::get_system_internal(size_t module_index, size_t system_index) const
    {
        // FIXME: range checks?
        const OffsetList& selected_module_systems = m_module_system_offsets[module_index];
        const size_t system_offset = selected_module_systems[system_index];

        return m_system_list[system_offset];
    }

    void SystemRegistry::register_system(Vadon::Core::SystemBase* system, size_t module_index, size_t system_index)
    {
        constexpr size_t c_invalid_offset = size_t(-1);

        if (module_index >= m_module_system_offsets.size())
        {
            m_module_system_offsets.resize(module_index + 1);
        }

        OffsetList& selected_module_systems = m_module_system_offsets[module_index];
        if (system_index >= selected_module_systems.size())
        {
            selected_module_systems.resize(system_index + 1, c_invalid_offset);
        }

        // FIXME: make sure we only register once?
        const size_t system_offset = m_system_list.size();
        assert(selected_module_systems[system_index] == c_invalid_offset); // Make sure we haven't tried to register the same system before
        selected_module_systems[system_index] = system_offset;

        m_system_list.push_back(system);
    }
}