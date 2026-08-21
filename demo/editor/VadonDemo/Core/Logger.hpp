#ifndef VADONDEMO_CORE_LOGGER_HPP
#define VADONDEMO_CORE_LOGGER_HPP
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>
namespace VadonDemo::Core
{
    class Logger : public Vadon::Core::DefaultLogger
    {
    public:
        Logger(::Vadon::Foundation::EditorPluginMessageSource source) : m_source(source) {}

        void log_message(std::string_view message) override;
        void log_warning(std::string_view message) override;
        void log_error(std::string_view message) override;
    protected:
        // NOTE: to be implemented by plugin interfaces
        virtual void dispatch_message_data(const char* data, size_t size) = 0;
    private:
        void dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type type, std::string_view message);

        ::Vadon::Foundation::EditorPluginMessageSource m_source;
    };
}
#endif