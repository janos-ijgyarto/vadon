#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/System/Singleton/Registry.hpp>
namespace Vadon::Utilities
{
    size_t SingletonRegistry::get_offset_internal(size_t module_index, size_t type_index) const
    {
        // FIXME: range checks?
        const OffsetList& selected_module_instances = m_module_instance_offsets[module_index];
        return selected_module_instances[type_index];
    }

    void SingletonRegistry::register_instance(Vadon::Utilities::SingletonBase* instance, size_t module_index, size_t type_index)
    {
        constexpr size_t c_invalid_offset = size_t(-1);

        if (module_index >= m_module_instance_offsets.size())
        {
            m_module_instance_offsets.resize(module_index + 1);
        }

        OffsetList& selected_module_instances = m_module_instance_offsets[module_index];
        if (type_index >= selected_module_instances.size())
        {
            selected_module_instances.resize(type_index + 1, c_invalid_offset);
        }

        // FIXME: make sure we only register once?
        const size_t instance_offset = m_instances.size();
        VADON_ASSERT(selected_module_instances[type_index] == c_invalid_offset, "System already registered!"); // Make sure we haven't tried to register the same system before
        selected_module_instances[type_index] = instance_offset;

        m_instances.push_back(instance);
    }
}