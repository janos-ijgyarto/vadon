#ifndef VADONEDITOR_NETWORK_TCP_CONNECTION_HPP
#define VADONEDITOR_NETWORK_TCP_CONNECTION_HPP
#include <VadonEditor/Common.hpp>
#include <asio.hpp>
namespace VadonEditor::Network::TCP
{
	class LoggingInterface
	{
	public:
		virtual void log_message(std::string_view message) = 0;
		virtual void log_error(std::string_view message) = 0;
	};

	class Connection : public std::enable_shared_from_this<Connection>
	{
	public:
		struct Observer
		{
			virtual void on_received(int connection_id, const char* data, size_t size);
			virtual void on_connection_closed(int connection_id);
		};

		static std::shared_ptr<Connection> create(asio::ip::tcp::socket&& socket, Observer& observer, LoggingInterface& logging_interface, int id = 0);

		void start_reading();
		void send(const char* data, size_t size);
		void close();
	private:
		VADONEDITORCOMMON_API Connection(asio::ip::tcp::socket&& socket, Observer& observer, LoggingInterface& logging_interface, int id);
		void do_read();
		void do_write();

		asio::ip::tcp::socket m_socket;
		asio::streambuf m_read_buffer;
		asio::streambuf m_write_buffer;
		std::mutex m_write_buffer_mutex;
		Observer& m_observer;
		bool m_is_reading;
		bool m_is_writing;
		bool m_is_closing;
		LoggingInterface& m_logging_interface;
		int m_id;
	};

}
#endif