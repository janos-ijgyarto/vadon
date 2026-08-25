#include <VadonEditor/Simulator/Network/Client.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

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

		constexpr size_t c_header_size = sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t);

		while (m_buffer.size() >= c_header_size)
		{
			// We have enough data for a header, check payload
			::Vadon::Foundation::EditorMessageReader message_reader(m_buffer.data(), m_buffer.size());

			const uint32_t available_buffer_size = static_cast<uint32_t>(m_buffer.size() - c_header_size);
			const uint32_t message_data_size = message_reader.get_message_data_size();
			if (message_data_size > available_buffer_size)
			{
				// Didn't get the rest of the message yet
				return;
			}

			// Offset data pointer from header to payload, then forward to the client
			m_client_interface.on_received(m_buffer.data(), m_buffer.size());
			m_buffer.erase(m_buffer.begin(), m_buffer.begin() + c_header_size + message_data_size);
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