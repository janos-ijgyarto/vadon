#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Settings.hpp>

#include <VadonEditor/Network/Message/Message.hpp>
#include <VadonEditor/Network/TCP/Server.hpp>
#include <VadonEditor/Network/TCP/Client.hpp>

#include <VadonEditor/Simulator/API/Plugin.hpp>
#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <QApplication>
#include <QTimer>
#include <QDebug>

namespace
{
	class TCPLoggingInterface : public VadonEditor::Network::TCP::LoggingInterface
	{
	public:
		void log_message(std::string_view message)
		{
			qInfo() << message;
		}

		void log_error(std::string_view message)
		{
			qCritical() << message;
		}
	};
}

namespace VadonEditor::Network
{
	struct NetworkSystem::Internal : public TCP::Server::Observer, public TCP::Client::Observer
	{
		Core::Application& m_application;
		TCPLoggingInterface m_logging_interface;

		asio::io_context m_io_context;
		std::unique_ptr<TCP::Server> m_tcp_server;
		std::unique_ptr<TCP::Client> m_tcp_client;
		QTimer* m_timer = nullptr;

		int m_connection_id;
		QByteArray m_buffer;

		Internal(Core::Application& application)
			: m_application(application)
			, m_connection_id(0)
		{
		}

		bool initialize()
		{
			if (m_application.get_settings().is_simulator == false)
			{
				m_tcp_server = std::make_unique<TCP::Server>(m_io_context, *this, m_logging_interface);
			}
			else
			{
				m_tcp_client = std::make_unique<TCP::Client>(m_io_context, *this, m_logging_interface);
			}

			return true;
		}

		// Server observer methods:
		void on_connection_accepted(int connection_id) override
		{
			Q_ASSERT_X(m_connection_id == 0, "Server", "More than one connection");
			m_connection_id = connection_id;
		}

		void on_server_received(int connection_id, const char* data, size_t size) override
		{
			Q_ASSERT_X(m_connection_id == connection_id, "Server", "Connection mismatch");
			internal_on_data_received(data, size);
		}

		void on_connection_closed(int connection_id) override
		{
			Q_ASSERT_X(m_connection_id == connection_id, "Server", "Connection mismatch");
			// TODO: assume app shut down?
			m_connection_id = 0;
		}

		// Client observer methods:
		void on_connected() override
		{
			qInfo() << "Client connected to editor!" << Qt::endl;
			m_application.get_plugin_manager().get_plugin()->editor_connected();
		}

		void on_client_received(const char* data, size_t size) override
		{
			internal_on_data_received(data, size);
		}

		void on_disconnected() override
		{
			qInfo() << "Client disconnected from editor!" << Qt::endl;
			m_application.get_plugin_manager().get_plugin()->editor_disconnected();
		}

		void internal_on_data_received(const char* data, size_t size)
		{
			m_buffer.append(data, size);

			while (m_buffer.size() >= sizeof(VadonEditor::Network::MessageHeader))
			{
				// We have enough data for a header, check payload
				const VadonEditor::Network::MessageHeader header = *reinterpret_cast<VadonEditor::Network::MessageHeader*>(m_buffer.data());
				if ((m_buffer.size() - sizeof(VadonEditor::Network::MessageHeader)) < header.size)
				{
					// Didn't get the rest of the packet yet
					return;
				}

				// Add header and payload to byte array and send out as a signal
				QByteArray message_data;
				message_data.append(m_buffer.data(), header.size + sizeof(VadonEditor::Network::MessageHeader));

				emit m_application.get_network_system().received_message(message_data);

				m_buffer.slice(sizeof(VadonEditor::Network::MessageHeader) + header.size);
			}
		}

		void start()
		{
			// TODO: get the values from the settings!
			constexpr asio::ip::port_type c_network_port = 13;

			if (m_tcp_server != nullptr)
			{
				m_tcp_server->listen(asio::ip::tcp::v4(), c_network_port);
				m_tcp_server->start_accepting_connections();
			}

			if (m_tcp_client != nullptr)
			{
				asio::error_code error_code;
				const asio::ip::address server_ip = asio::ip::make_address("127.0.0.1", error_code);

				// TODO: use error code?

				m_tcp_client->connect(asio::ip::tcp::endpoint{ server_ip, c_network_port });
			}

			// Set a timer to poll the IO context (this means we don't block the network thread and can receive other signals)
			m_timer = new QTimer(&m_application.get_network_system());
			connect(m_timer, &QTimer::timeout, &m_application.get_network_system(), &NetworkSystem::poll);
			m_timer->start(200);
		}

		void close()
		{
			if (m_tcp_server != nullptr)
			{
				m_tcp_server->close();
			}

			if (m_tcp_client != nullptr)
			{
				m_tcp_client->disconnect();
			}

			m_io_context.run(); // Run to make sure we have no more outstanding work
		}

		void poll()
		{
			m_io_context.poll();
		}

		void send_message(const QByteArray& data)
		{
			if (m_tcp_server != nullptr && (m_connection_id != 0))
			{
				m_tcp_server->send(m_connection_id, data.data(), data.size());
			}

			if (m_tcp_client != nullptr)
			{
				m_tcp_client->send(data.data(), data.size());
			}
		}
	};

	NetworkSystem::NetworkSystem(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{
	}

	NetworkSystem::~NetworkSystem()
	{

	}

	void NetworkSystem::start()
	{
		m_internal->start();
	}

	void NetworkSystem::close()
	{
		m_internal->close();
	}

	void NetworkSystem::poll()
	{
		m_internal->poll();
	}

	void NetworkSystem::send_message(const QByteArray& data)
	{
		m_internal->send_message(data);
	}

	bool NetworkSystem::initialize()
	{
		return m_internal->initialize();
	}
}