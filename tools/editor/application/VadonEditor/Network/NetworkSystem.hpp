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
	class NetworkSystem : public QObject
	{
		Q_OBJECT
	public:
		NetworkSystem(Core::Application& application);
		~NetworkSystem();
	signals:
		void received_message(const QByteArray& data);
	public slots:
		void start();
		void close();
	private slots:
		void poll();
		void send_message(const QByteArray& data);
	private:
		bool initialize();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif