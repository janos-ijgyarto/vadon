#include <VadonEditor/Network/MessageSystem.hpp>

namespace VadonEditor::Network
{
	void MessageSystem::send_message(const QByteArray& data)
	{
		// Just emit the signal so it gets sent to the Server
		emit message_dispatched(data);
	}

	void MessageSystem::internal_received_message(const QByteArray& data)
	{
		// Emit signal so objects on the main thread can process it
		emit received_message(data);
	}
}