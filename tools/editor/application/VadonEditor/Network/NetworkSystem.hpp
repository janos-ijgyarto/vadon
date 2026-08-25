#ifndef VADONEDITOR_NETWORK_NETWORKSYSTEM_HPP
#define VADONEDITOR_NETWORK_NETWORKSYSTEM_HPP
#include <QObject>
#include <memory>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Network
{
	class NetworkThreadWorker : public QObject
	{
		Q_OBJECT
	public:
		~NetworkThreadWorker();
	signals:
		void received_message(const QByteArray& data);

		void connected();
		void disconnected();
	private slots:
		void start();
		void close();

		void send_message(const QByteArray& data);
	private:
		NetworkThreadWorker(Core::Application& application);

		bool initialize();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend class NetworkSystem;
	};

	class MessageSerializer;

	class NetworkSystem : public QObject
	{
		Q_OBJECT
	public:
		NetworkSystem(Core::Application& application);
		~NetworkSystem();

		void send_message(QByteArrayView data);
		void send_message(const MessageSerializer& message_serializer);
	signals:
		void received_message(const QByteArray& data);

		void connected_to_server();
		void disconnected_from_server();
	private slots:
		void internal_received_message(const QByteArray& data);
		void internal_disconnected_from_server();
	private:
		bool initialize();
		void shutdown();

		void run_network();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif