#ifndef VADONEDITOR_CORE_LOGGER_HPP
#define VADONEDITOR_CORE_LOGGER_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>
namespace VadonEditor::Core
{
    // A simple logger interface that will also convert the log message to a package for the editor app
    class Logger : public Vadon::Core::DefaultLogger
    {
    public:
        Logger(::Vadon::Foundation::EditorPluginMessageSource source) : m_source(source) {}

        VADONEDITOR_API void log_message(std::string_view message) override;
        VADONEDITOR_API void log_warning(std::string_view message) override;
        VADONEDITOR_API void log_error(std::string_view message) override;
    protected:
        // NOTE: to be implemented by plugin interfaces
        virtual void dispatch_message_data(const char* data, size_t size) = 0;
    private:
        void dispatch_network_log_message(::Vadon::Foundation::EditorPluginMessageLog::Type type, std::string_view message);

        ::Vadon::Foundation::EditorPluginMessageSource m_source;
    };
}
#endif