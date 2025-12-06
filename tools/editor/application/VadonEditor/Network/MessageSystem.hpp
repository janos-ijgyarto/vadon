#ifndef VADONEDITOR_NETWORK_MESSAGESYSTEM_HPP
#define VADONEDITOR_NETWORK_MESSAGESYSTEM_HPP
#include <QObject>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Network
{
	// NOTE: utility object that allows code outside QObjects to dispatch and receive network messages
	// while making use of the thread-safety of Qt signals and slots
	class MessageSystem : public QObject
	{
		Q_OBJECT
	public:
		void send_message(const QByteArray& data);
	signals:
		void received_message(const QByteArray& data);
		void message_dispatched(const QByteArray& data);
	private slots:
		void internal_received_message(const QByteArray& data);

		friend Core::Application;
	};
}
#endif