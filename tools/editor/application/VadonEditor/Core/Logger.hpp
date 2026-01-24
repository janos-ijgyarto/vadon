#ifndef VADONEDITOR_CORE_LOGGER_HPP
#define VADONEDITOR_CORE_LOGGER_HPP
#include <QObject>
namespace VadonEditor::Core
{
    // NOTE: utility object that sends QDebug messages to the UI via signals (ensures thread safety)
    class Logger : public QObject
    {
        Q_OBJECT
    signals:
        void message_logged(const QString& message);

    private:
        void handle_message(QtMsgType type, const QMessageLogContext& context, const QString& message);
        void log_message(QtMsgType type, const QString& message_string);

        friend class Application;
    };
}
#endif