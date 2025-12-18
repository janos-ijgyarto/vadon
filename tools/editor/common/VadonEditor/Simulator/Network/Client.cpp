#include <VadonEditor/Simulator/Network/Client.hpp>

#include <VadonEditor/Network/Message/Message.hpp>

namespace VadonEditor::Simulator::Network
{
	Client::Client(VadonEditor::Network::TCP::LoggingInterface& logging_interface, ClientInterface& client_interface)
		: m_tcp_client(m_io_context, *this, logging_interface)
		, m_client_interface(client_interface)
	{
	}

	void Client::start(const asio::ip::address& ip_address, asio::ip::port_type port)
	{
		// Start the thread that handles the network
		m_thread = std::jthread(&thread_func, this, ip_address, port);
	}

	void Client::stop()
	{
		m_thread.request_stop();

		// Wait for thread to join (we want to halt network comms before we start cleaning up all the potential listeners)
		m_thread.join();
	}

	void Client::on_connected()
	{
		m_client_interface.on_connected();
	}

	void Client::on_client_received(const char* data, size_t size)
	{
		const size_t prev_size = m_buffer.size();
		m_buffer.resize(prev_size + size);
		memcpy(m_buffer.data() + prev_size, data, size);

		while (m_buffer.size() >= sizeof(VadonEditor::Network::MessageHeader))
		{
			// We have enough data for a header, check payload
			const VadonEditor::Network::MessageHeader* header = reinterpret_cast<VadonEditor::Network::MessageHeader*>(m_buffer.data());
			
			if ((m_buffer.size() - sizeof(VadonEditor::Network::MessageHeader)) < header->size)
			{
				// Didn't get the rest of the message yet
				return;
			}

			// Offset data pointer from header to payload, then forward to the client
			m_client_interface.on_received(*header, m_buffer.data() + sizeof(VadonEditor::Network::MessageHeader));
			m_buffer.erase(m_buffer.begin(), m_buffer.begin() + sizeof(VadonEditor::Network::MessageHeader) + header->size);
		}
	}

	void Client::on_disconnected()
	{
		m_client_interface.on_disconnected();
	}

	void Client::thread_func(std::stop_token stop_token, Client* self, asio::ip::address ip_address, asio::ip::port_type port)
	{
		self->m_tcp_client.connect({ ip_address, port });

		using namespace std::chrono_literals;
		while (stop_token.stop_requested() == false)
		{
			self->m_io_context.poll();
			std::this_thread::sleep_for(300ms);
		}

		self->m_tcp_client.disconnect();

		// Run IO context to make sure everything is completed
		self->m_io_context.run();
	}
}