#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Core/Logger.hpp>
#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace Vadon::Core
{
	EngineEnvironment* EngineEnvironment::s_instance = nullptr;

    struct EngineEnvironment::Internal
    {
        Vadon::ECS::ComponentRegistry m_component_registry;
        Vadon::Scene::ResourceRegistry m_resource_registry;
        Vadon::Utilities::TypeRegistry m_type_registry;
        DefaultLogger m_default_logger;

        LoggerInterface* m_logger;

        Internal()
        {
            set_logger(nullptr);
        }

        void set_logger(LoggerInterface* logger)
        {
            if (logger != nullptr)
            {
                m_logger = logger;
            }
            else
            {
                m_logger = &m_default_logger;
            }
        }
    };

	EngineEnvironment::EngineEnvironment()
		: m_internal(std::make_unique<Internal>())
	{
	}

	EngineEnvironment::~EngineEnvironment() = default;

    bool EngineEnvironment::is_initialized()
    {
        return s_instance != nullptr;
    }

    ::Vadon::ECS::ComponentRegistry& EngineEnvironment::get_component_registry()
    {
        return s_instance->m_internal->m_component_registry;
    }

    ::Vadon::Scene::ResourceRegistry& EngineEnvironment::get_resource_registry()
    {
        return s_instance->m_internal->m_resource_registry;
    }

    ::Vadon::Utilities::TypeRegistry& EngineEnvironment::get_type_registry()
    {
        return s_instance->m_internal->m_type_registry;
    }

    LoggerInterface& EngineEnvironment::get_logger()
    {
        return *s_instance->m_internal->m_logger;
    }

    void EngineEnvironment::set_logger(LoggerInterface* logger)
    {
        s_instance->m_internal->set_logger(logger);
    }

	void EngineEnvironment::initialize(EngineEnvironment& instance)
	{
        VADON_ASSERT((s_instance == nullptr) || (s_instance == &instance), "Attempted to set different engine environment instances!\n");
		s_instance = &instance;
	}
}