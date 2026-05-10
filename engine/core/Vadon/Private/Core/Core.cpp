#include <Vadon/Private/Core/Core.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Private/Render/RenderSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

namespace Vadon::Core
{
    void init_engine_environment(EngineEnvironment& environment)
    {
        // Make sure all modules have the environment initialized with the same instance
        EngineEnvironment::initialize(environment);

        Vadon::Private::Render::RenderSystem::init_engine_environment(environment);
        Vadon::Private::Render::GraphicsAPIBase::init_engine_environment(environment);
    }

    void register_engine_types()
    {
        Vadon::Private::Core::EngineCore::register_types();
    }

    void register_engine_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
    {
        Vadon::Private::Core::EngineCore::register_type_metadata(metadata_registry);
    }

    EngineCorePtr create_engine_core()
    {
        return std::make_unique<Vadon::Private::Core::EngineCore>();
    }
}

namespace Vadon::Private::Core
{
    EngineCore::EngineCore()
        : m_file_system(*this)
        , m_task_system(*this) 
        , m_render_system(*this)
        , m_resource_system(*this)
        , m_scene_system(*this)
    {
    }

    EngineCore::~EngineCore() = default;

    void EngineCore::register_types()
    {
        Vadon::Private::Model::ResourceSystem::register_types();
        Vadon::Private::Model::SceneSystem::register_types();
    }

    void EngineCore::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
    {
        // Register metadata for base types
        // FIXME: is this even needed? These types are already expected to be handled separately
        {
            {
                Vadon::Utilities::TypeMetadata int_metadata(metadata_registry, VADON_GET_TYPE_UUID(int));
                int_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "integer");
            }

            {
                Vadon::Utilities::TypeMetadata uint_metadata(metadata_registry, VADON_GET_TYPE_UUID(uint32_t));
                uint_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "unsigned integer");
            }

            {
                Vadon::Utilities::TypeMetadata float_metadata(metadata_registry, VADON_GET_TYPE_UUID(float));
                float_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "float");
            }

            {
                Vadon::Utilities::TypeMetadata bool_metadata(metadata_registry, VADON_GET_TYPE_UUID(bool));
                bool_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "boolean");
            }

            {
                Vadon::Utilities::TypeMetadata string_metadata(metadata_registry, VADON_GET_TYPE_UUID(std::string));
                string_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "string");
            }

            {
                Vadon::Utilities::TypeMetadata vec2_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::Vector2));
                vec2_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vector2");
            }

            {
                Vadon::Utilities::TypeMetadata vec2i_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::Vector2i));
                vec2i_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vector2i");
            }

            {
                Vadon::Utilities::TypeMetadata vec3_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::Vector3));
                vec3_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vector3");
            }

            {
                Vadon::Utilities::TypeMetadata vec3i_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::Vector3i));
                vec3i_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vector3i");
            }

            {
                Vadon::Utilities::TypeMetadata vec4_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::Vector4));
                vec4_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vector4");
            }

            {
                Vadon::Utilities::TypeMetadata color_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Math::ColorRGBA));
                color_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "ColorRGBA");
            }

            {
                Vadon::Utilities::TypeMetadata uuid_metadata(metadata_registry, VADON_GET_TYPE_UUID(::Vadon::Foundation::UUID));
                uuid_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "UUID");
            }
        }

        Vadon::Private::Model::ResourceSystem::register_type_metadata(metadata_registry);
        Vadon::Private::Model::SceneSystem::register_type_metadata(metadata_registry);
    }

    bool EngineCore::initialize(const Vadon::Core::CoreConfiguration& config)
    {
        Vadon::Core::Logger::log_message("Vadon engine core initializing.\n");
        constexpr const char* c_failure_message = "Vadon engine core initialization failed!\n";

        m_config = config;

        if (m_file_system.initialize() == false)
        {
            Vadon::Core::Logger::log_message(c_failure_message);
            return false;
        }

        if (m_task_system.initialize() == false)
        {
            Vadon::Core::Logger::log_message(c_failure_message);
            return false;
        }

        // TODO: use configuration to select graphics backend
        m_graphics_api = Render::GraphicsAPIBase::get_graphics_api(*this);
        if (m_graphics_api->initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_render_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_resource_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_scene_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        Vadon::Core::Logger::log_message("Vadon initialized successfully.\n");
        return true;
    }

    void EngineCore::shutdown()
    {
        Vadon::Core::Logger::log_message("Vadon shutting down.\n");

        m_scene_system.shutdown();
        m_resource_system.shutdown();
        m_task_system.shutdown();
        m_render_system.shutdown();
        if (m_graphics_api != nullptr)
        {
            m_graphics_api->shutdown();
        }

        Vadon::Core::Logger::log_message("Vadon shut down successfully.\n");
    }
}