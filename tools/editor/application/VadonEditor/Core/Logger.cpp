#include <VadonEditor/Core/Logger.hpp>

namespace VadonEditor::Core
{
	void Logger::internal_log_message(const QString& message)
	{
		emit log_message(message);
	}
}