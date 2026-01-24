#include <VadonEditor/Core/Logger.hpp>

#include <QMutex>

namespace
{
	const char* get_message_type_header(QtMsgType type)
	{
		switch (type)
		{
		case QtDebugMsg:
			return "[DEBUG]";
		case QtInfoMsg:
			return "[INFO]";
		case QtWarningMsg:
			return "[WARNING]";
		case QtCriticalMsg:
			return "[CRITICAL]";
		case QtFatalMsg:
			return "[FATAL]";
		default:
			return "[INVALID]";
		}
	}
}

namespace VadonEditor::Core
{
	void Logger::handle_message(QtMsgType type, const QMessageLogContext& /*context*/, const QString& message)
	{
		// TODO: use context?
		QString message_string = QString("%1 %2").arg(get_message_type_header(type)).arg(message);
		log_message(type, message_string);
	}

	void Logger::log_message(QtMsgType type, const QString& message_string)
	{
		static QMutex mutex;
		{
			QMutexLocker lock(&mutex);
			switch (type)
			{
			case QtDebugMsg:
			case QtInfoMsg:
				fprintf(stdout, "%s\n", qPrintable(message_string));
				break;
			case QtWarningMsg:
			case QtCriticalMsg:
				fprintf(stderr, "%s\n", qPrintable(message_string));
				break;
			case QtFatalMsg:
				// On fatal, we abort the application, so we flush the stream beforehand
				fprintf(stderr, "%s\n", qPrintable(message_string));
				fflush(stderr);
				abort();
			}

			fflush(stderr);
			fflush(stdout);
		}
		emit message_logged(message_string);
	}
}