#include <VadonEditor/Network/TCP/Client.hpp>

namespace VadonEditor::Network::TCP
{
    void Client::Observer::on_connected() {}

    void Client::Observer::on_client_received([[maybe_unused]] const char* data, [[maybe_unused]] size_t size) {}

    void Client::Observer::on_disconnected() {}

    Client::Client(asio::io_context& io_context, Observer& observer, LoggingInterface& logging_interface)
        : m_io_context{ io_context }
        , m_connection{}
        , m_observer{ observer }
        , m_logging_interface(logging_interface)
    {
    }

    void Client::connect(const asio::ip::tcp::endpoint& endpoint) 
    {
        if (m_connection) 
        {
            return;
        }
        auto socket = std::make_shared<asio::ip::tcp::socket>(m_io_context);
        socket->async_connect(endpoint,
            [this, socket](const auto& error)
            {
                if (error) 
                {
                    m_logging_interface.log_error(std::format("Client::connect() error: {}", error.message()));
                    m_observer.on_disconnected();
                    return;
                }
                m_connection = Connection::create(std::move(*socket), *this, m_logging_interface);
                m_connection->start_reading();
                m_logging_interface.log_message("Client connected");
                m_observer.on_connected();
            }
        );
    }

    void Client::send(const char* data, size_t size) {
        if (m_connection == nullptr)
        {
            m_logging_interface.log_error("Client::send() error: no connection");
            return;
        }
        m_connection->send(data, size);
    }

    void Client::disconnect() 
    {
        if (m_connection != nullptr) 
        {
            m_connection->close();
        }
    }

    void Client::on_received([[maybe_unused]] int connection_id, const char* data, size_t size) {
        m_observer.on_client_received(data, size);
    }

    void Client::on_connection_closed([[maybe_unused]] int connection_id) 
    {
        if (m_connection) 
        {
            m_connection.reset();
            m_logging_interface.log_message("Client disconnected");
            m_observer.on_disconnected();
        }
    }
}