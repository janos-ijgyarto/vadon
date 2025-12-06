#ifndef VADONEDITOR_NETWORK_TCP_SERVER_HPP
#define VADONEDITOR_NETWORK_TCP_SERVER_HPP
#include <VadonEditor/Network/TCP/Connection.hpp>
namespace VadonEditor::Network::TCP
{
	class Server : private Connection::Observer
	{
	public:
		struct Observer 
		{
			VADONEDITORCOMMON_API virtual void on_connection_accepted(int connection_id);
			VADONEDITORCOMMON_API virtual void on_server_received(int connection_id, const char* data, size_t size);
			VADONEDITORCOMMON_API virtual void on_connection_closed(int connection_id);
		};

		VADONEDITORCOMMON_API Server(asio::io_context& io_context, Observer& observer, LoggingInterface& logging_interface);

		VADONEDITORCOMMON_API bool listen(const asio::ip::tcp& protocol, uint16_t port);
		VADONEDITORCOMMON_API void start_accepting_connections();
		VADONEDITORCOMMON_API void send(int connection_id, const char* data, size_t size);
		VADONEDITORCOMMON_API void close();

	private:
		void do_accept();
		void on_received(int connection_id, const char* data, size_t size) override;
		void on_connection_closed(int connection_id) override;

		asio::ip::tcp::acceptor m_acceptor;
		std::unordered_map<int, std::shared_ptr<Connection>> m_connections;
		Observer& m_observer;
		int m_connection_counter;
		bool m_is_accepting;
		bool m_is_closing;
		LoggingInterface& m_logging_interface;
	};
}
#endif