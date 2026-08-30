#include <VadonEditor/Core/Logger.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

namespace VadonEditor::Core
{
    void Logger::log_message(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_message(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type::LOG_INFO, message);
    }

    void Logger::log_warning(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_warning(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type::LOG_WARNING, message);
    }

    void Logger::log_error(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_error(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type::LOG_ERROR, message);
    }

    void Logger::dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type type, std::string_view message)
    {
        // Trim trailing newline
        // FIXME: make this more robust!
        std::string_view message_trimmed = message;
        if (message_trimmed.back() == '\n')
        {
            message_trimmed = message.substr(0, message.length() - 1);
        }

        if (message_trimmed.empty())
        {
            return;
        }

        ::Vadon::Foundation::EditorPluginMessageLog log_message;
        log_message.message_type = ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_LOG;
        log_message.plugin_type = m_source;
        log_message.log_type = type;
        log_message.length = static_cast<::Vadon::Foundation::uint32>(message_trimmed.size());

        VadonEditor::Network::MessageSerializer message_serializer;
        char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLUGIN, sizeof(::Vadon::Foundation::EditorPluginMessageLog) + message_trimmed.size());

        memcpy(message_data, &log_message, sizeof(::Vadon::Foundation::EditorPluginMessageLog));
        memcpy(message_data + sizeof(::Vadon::Foundation::EditorPluginMessageLog), message_trimmed.data(), message_trimmed.size());

        dispatch_message_data(message_serializer.get_buffer().data(), message_serializer.get_buffer().size());
    }
}