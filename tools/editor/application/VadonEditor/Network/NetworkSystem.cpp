#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>

#include <VadonEditor/Network/TCP/Server.hpp>
#include <VadonEditor/Network/TCP/Client.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>

#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>

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
	struct NetworkThreadWorker::Internal : public TCP::Server::Observer, public TCP::Client::Observer
	{
		Core::Application& m_application;
		NetworkThreadWorker& m_thread_worker;

		TCPLoggingInterface m_logging_interface;

		asio::io_context m_io_context;
		std::unique_ptr<TCP::Server> m_tcp_server;
		std::unique_ptr<TCP::Client> m_tcp_client;
		QTimer* m_timer = nullptr;

		int m_connection_id;
		QByteArray m_buffer;

		Internal(Core::Application& application, NetworkThreadWorker& thread_worker)
			: m_application(application)
			, m_thread_worker(thread_worker)
			, m_connection_id(0)
		{

		}

		bool initialize()
		{
			if (m_application.get_command_line_parameters().is_simulator == false)
			{
				m_tcp_server = std::make_unique<TCP::Server>(m_io_context, *this, m_logging_interface);
			}
			else
			{
				m_tcp_client = std::make_unique<TCP::Client>(m_io_context, *this, m_logging_interface);
			}

			return true;
		}

		void shutdown()
		{
			// TODO: anything?
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
			qInfo() << "Client connected to editor!";
			emit m_thread_worker.connected();
		}

		void on_client_received(const char* data, size_t size) override
		{
			internal_on_data_received(data, size);
		}

		void on_disconnected() override
		{
			qInfo() << "Client disconnected from editor!";
			emit m_thread_worker.disconnected();
		}

		void internal_on_data_received(const char* data, size_t size)
		{
			m_buffer.append(data, size);

			while (m_buffer.size() >= ::Vadon::Foundation::EditorMessageReader::c_header_size)
			{
				// We have enough data for a header, check payload
				::Vadon::Foundation::EditorMessageReader message_reader(m_buffer.constData(), m_buffer.size());
				const ::Vadon::Foundation::uint32 message_data_size = message_reader.get_message_data_size();
				if ((m_buffer.size() - ::Vadon::Foundation::EditorMessageReader::c_header_size) < message_data_size)
				{
					// Didn't get the rest of the packet yet
					return;
				}

				// Add header and payload to byte array and send out as a signal
				QByteArray message_data;
				message_data.append(m_buffer.constData(), ::Vadon::Foundation::EditorMessageReader::c_header_size + message_data_size);

				emit m_thread_worker.received_message(message_data);

				m_buffer.slice(::Vadon::Foundation::EditorMessageReader::c_header_size + message_data_size);
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
			m_timer = new QTimer();
			connect(m_timer, &QTimer::timeout, 
				[this]()
				{
					poll();
				}
			);
			m_timer->start(200);
			qDebug() << "Server started";
		}

		void close()
		{
			if (m_timer != nullptr)
			{
				m_timer->stop();
				m_timer->deleteLater();
			}

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
	};

	NetworkThreadWorker::~NetworkThreadWorker() = default;

	void NetworkThreadWorker::start()
	{
		m_internal->start();
	}

	void NetworkThreadWorker::close()
	{
		m_internal->close();
	}

	void NetworkThreadWorker::send_message(const QByteArray& data)
	{
		if (m_internal->m_tcp_server != nullptr && (m_internal->m_connection_id != 0))
		{
			m_internal->m_tcp_server->send(m_internal->m_connection_id, data.data(), data.size());
		}

		if (m_internal->m_tcp_client != nullptr)
		{
			m_internal->m_tcp_client->send(data.data(), data.size());
		}
	}

	NetworkThreadWorker::NetworkThreadWorker(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application, *this))
	{

	}

	bool NetworkThreadWorker::initialize()
	{
		return m_internal->initialize();
	}

	void NetworkThreadWorker::shutdown()
	{
		m_internal->shutdown();
	}

	struct NetworkSystem::Internal
	{
		Core::Application& m_application;

		QThread m_worker_thread;
		NetworkThreadWorker m_worker_object;

		Internal(Core::Application& application)
			: m_application(application)
			, m_worker_object(application)
		{
		}

		bool initialize()
		{
			if (m_worker_object.initialize() == false)
			{
				return false;
			}

			m_worker_object.moveToThread(&m_worker_thread);

			QObject::connect(&m_worker_thread, &QThread::finished, &m_worker_object, &NetworkThreadWorker::close);
			
			NetworkSystem* network_system = &m_application.get_network_system();
			QObject::connect(&m_worker_object, &NetworkThreadWorker::received_message, network_system, &NetworkSystem::internal_received_message);
			QObject::connect(&m_worker_object, &NetworkThreadWorker::connected, network_system, &NetworkSystem::connected_to_server);
			QObject::connect(&m_worker_object, &NetworkThreadWorker::disconnected, network_system, &NetworkSystem::disconnected_from_server);

			return true;
		}

		void shutdown()
		{
			if (m_worker_thread.isRunning() == true)
			{
				// Wait for the worker thread to quit
				m_worker_thread.quit();
				m_worker_thread.wait();
			}
		}

		void send_message(const QByteArray& data)
		{
			QMetaObject::invokeMethod(&m_worker_object, &NetworkThreadWorker::send_message, Qt::ConnectionType::QueuedConnection, data);
		}

		void run_network()
		{
			if (m_worker_thread.isRunning() == true)
			{
				// Worker is already running
				return;
			}

			// First start the worker thread
			m_worker_thread.start();

			// Invoke start with blocking queued connection
			// This ensures we only return once the thread is started and the worker is intialized
			QMetaObject::invokeMethod(&m_worker_object, &NetworkThreadWorker::start, Qt::ConnectionType::BlockingQueuedConnection);
		}
	};

	NetworkSystem::NetworkSystem(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{
	}

	NetworkSystem::~NetworkSystem()
	{

	}

	void NetworkSystem::send_message(const QByteArray& data)
	{
		m_internal->send_message(data);
	}

	void NetworkSystem::internal_received_message(const QByteArray& data)
	{
		emit received_message(data);
	}

	bool NetworkSystem::initialize()
	{
		return m_internal->initialize();
	}

	void NetworkSystem::shutdown()
	{
		m_internal->shutdown();
	}

	void NetworkSystem::run_network()
	{
		m_internal->run_network();
	}
}