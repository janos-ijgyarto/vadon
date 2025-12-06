#include <VadonEditor/Network/TCP/Connection.hpp>

namespace 
{
    constexpr auto c_read_buffer_size{ 1024 };
}

namespace VadonEditor::Network::TCP
{
    void Connection::Observer::on_received([[maybe_unused]] int connection_id, [[maybe_unused]] const char* data, [[maybe_unused]] const size_t size) {}

    void Connection::Observer::on_connection_closed([[maybe_unused]] int connection_id) {}

    Connection::Connection(asio::ip::tcp::socket&& socket, Observer& observer, LoggingInterface& logging_interface, int id)
        : m_socket{ std::move(socket) }
        , m_read_buffer{}
        , m_write_buffer{}
        , m_write_buffer_mutex{}
        , m_observer{ observer }
        , m_is_reading{ false }
        , m_is_writing{ false }
        , m_is_closing{ false }
        , m_logging_interface(logging_interface)
        , m_id{ id }
    {
    }

    std::shared_ptr<Connection> Connection::create(asio::ip::tcp::socket&& socket, Observer& observer, LoggingInterface& logging_interface, int id)
    {
        return std::shared_ptr<Connection>(new Connection{ std::move(socket), observer, logging_interface, id });
    }

    void Connection::start_reading()
    {
        if (m_is_reading == false)
        {
            do_read();
        }
    }

    void Connection::send(const char* data, size_t size)
    {
        std::lock_guard<std::mutex> guard{ m_write_buffer_mutex };
        std::ostream bufferStream{ &m_write_buffer };
        bufferStream.write(data, size);
        if (m_is_writing == false)
        {
            do_write();
        }
    }

    void Connection::close()
    {
        if (m_is_closing == true)
        {
            return;
        }
        m_is_closing = true;
        try
        {
            m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
            m_socket.close();
        }
        catch (const std::exception& e)
        {
            m_logging_interface.log_error(std::format("Connection::close() exception: {}", e.what()));
            return;
        }
        m_observer.on_connection_closed(m_id);
    }

    void Connection::do_read()
    {
        m_is_reading = true;
        auto buffers{ m_read_buffer.prepare(c_read_buffer_size) };
        auto self{ shared_from_this() };
        m_socket.async_read_some(buffers, [this, self](const auto& error, auto bytes_transferred)
            {
                if (error)
                {
                    m_logging_interface.log_error(std::format("Connection::do_read() error: {}", error.message()));
                    close();
                    return;
                }
                m_read_buffer.commit(bytes_transferred);
                m_observer.on_received(m_id, static_cast<const char*>(m_read_buffer.data().data()), bytes_transferred);
                m_read_buffer.consume(bytes_transferred);
                do_read();
            }
        );
    }

    void Connection::do_write()
    {
        m_is_writing = true;
        auto self{ shared_from_this() };
        m_socket.async_write_some(m_write_buffer.data(), [this, self](const auto& error, auto bytes_transferred)
            {
                if (error)
                {
                    m_logging_interface.log_error(std::format("Connection::do_write() error: {}", error.message()));
                    close();
                    return;
                }
                std::lock_guard<std::mutex> guard{ m_write_buffer_mutex };
                m_write_buffer.consume(bytes_transferred);
                if (m_write_buffer.size() == 0)
                {
                    m_is_writing = false;
                    return;
                }
                do_write();
            }
        );
    }
}
