#include <VadonEditor/Network/TCP/Server.hpp>

namespace VadonEditor::Network::TCP
{
    void Server::Observer::on_connection_accepted([[maybe_unused]] int connection_id) {}

    void Server::Observer::on_server_received([[maybe_unused]] int connection_id, [[maybe_unused]] const char* data, [[maybe_unused]] const size_t size) {}

    void Server::Observer::on_connection_closed([[maybe_unused]] int connection_id) {}

    Server::Server(asio::io_context& io_context, Observer& observer, LoggingInterface& logging_interface)
        : m_acceptor{ io_context }
        , m_connections{}
        , m_observer{ observer }
        , m_connection_counter{ 0 }
        , m_is_accepting{ false }
        , m_is_closing{ false } 
        , m_logging_interface(logging_interface)
    {
    }

    bool Server::listen(const asio::ip::tcp& protocol, uint16_t port)
    {
        try
        {
            m_acceptor.open(protocol);
            m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind({ protocol, port });
            m_acceptor.listen(asio::socket_base::max_listen_connections);
        }
        catch (const std::exception& e) 
        {
            m_logging_interface.log_error(std::format("Server::listen() exception: {}", e.what()));
            return false;
        }
        return true;
    }

    void Server::start_accepting_connections()
    {
        if (m_is_accepting == false)
        {
            do_accept();
        }
    }

    void Server::send(int connection_id, const char* data, size_t size) 
    {
        if (m_connections.count(connection_id) == 0)
        {
            m_logging_interface.log_error("Server::send() error: connection not found.");
            return;
        }
        m_connections.at(connection_id)->send(data, size);
    }

    void Server::close() 
    {
        m_is_closing = true;
        m_acceptor.cancel();
        for (const auto& connection : m_connections)
        {
            connection.second->close();
        }
        m_connections.clear();
        m_is_closing = false;
        m_logging_interface.log_message("Server disconnected.");
    }

    void Server::do_accept() 
    {
        m_is_accepting = true;
        m_acceptor.async_accept(
            [this](const auto& error, auto socket) 
            {
                if (error)
                {
                    m_logging_interface.log_error(std::format("Server::do_accept() error: {}", error.message()));
                    m_is_accepting = false;
                    return;
                }
                else 
                {
                    ++m_connection_counter;
                    const int connection_id = m_connection_counter;

                    auto connection{ Connection::create(std::move(socket), *this, m_logging_interface, connection_id) };
                    connection->start_reading();
                    m_connections.insert({ connection_id, std::move(connection) });
                    m_logging_interface.log_message("Server accepted connection.");
                    m_observer.on_connection_accepted(connection_id);
                }
                do_accept();
            }
        );
    }

    void Server::on_received(int connection_id, const char* data, size_t size)
    {
        m_observer.on_server_received(connection_id, data, size);
    }

    void Server::on_connection_closed(int connection_id)
    {
        if (m_is_closing) 
        {
            return;
        }

        if (m_connections.erase(connection_id) > 0)
        {
            m_logging_interface.log_message("Server removed connection.");
            m_observer.on_connection_closed(connection_id);
        }
    }
}