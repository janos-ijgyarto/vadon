#ifndef VADONEDITOR_SIMULATOR_NETWORK_CLIENT_HPP
#define VADONEDITOR_SIMULATOR_NETWORK_CLIENT_HPP
#include <VadonEditor/Common.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>
#include <VadonEditor/Network/TCP/Client.hpp>
namespace VadonEditor::Simulator::Network
{
	// NOTE: these are some utility classes for projects that want to create their own
	// simulator to be run by the main Editor app
	class ClientInterface
	{
	public:
		virtual void on_connected() = 0;
		virtual void on_received(const VadonEditor::Network::MessageHeader& header, const char* data) = 0;
		virtual void on_disconnected() = 0;
	};

	class Client : public VadonEditor::Network::TCP::Client::Observer
	{
	public:
		VADONEDITORCOMMON_API Client(VadonEditor::Network::TCP::LoggingInterface& logging_interface, ClientInterface& client_interface);

		VADONEDITORCOMMON_API void start(const asio::ip::address& ip_address, asio::ip::port_type port);
		VADONEDITORCOMMON_API void stop();

		void send_message(const char* data, size_t size) { m_tcp_client.send(data, size); }

		void on_connected() override;
		void on_client_received(const char* data, size_t size) override;
		void on_disconnected() override;
	private:
		static void thread_func(std::stop_token stop_token, Client* self, asio::ip::address ip_address, asio::ip::port_type port);

		ClientInterface& m_client_interface;

		asio::io_context m_io_context;
		VadonEditor::Network::TCP::Client m_tcp_client;

		std::jthread m_thread;

		std::vector<char> m_buffer;
	};
}
#endif