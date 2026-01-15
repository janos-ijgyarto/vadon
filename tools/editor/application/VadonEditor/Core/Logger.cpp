#include <VadonEditor/Core/Logger.hpp>

namespace VadonEditor::Core
{
	void Logger::log_message(const QString& message)
	{
		emit message_logged(message);
	}
}