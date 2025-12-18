#ifndef VADONEDITOR_CORE_LOGGER_HPP
#define VADONEDITOR_CORE_LOGGER_HPP
#include <QObject>
namespace VadonEditor::Core
{
    // NOTE: utility object that sends QDebug messages to the UI via signals
    class Logger : public QObject
    {
        Q_OBJECT
    signals:
        void log_message(const QString& message);

    private:
        void internal_log_message(const QString& message);

        friend class Application;
    };
}
#endif