#ifndef VADONEDITOR_NETWORK_TCP_CLIENT_HPP
#define VADONEDITOR_NETWORK_TCP_CLIENT_HPP
#include <VadonEditor/Network/TCP/Connection.hpp>
namespace VadonEditor::Network::TCP
{
    class Client : private Connection::Observer
    {
    public:
        struct Observer 
        {
            VADONEDITORCOMMON_API virtual void on_connected();
            VADONEDITORCOMMON_API virtual void on_client_received(const char* data, size_t size);
            VADONEDITORCOMMON_API virtual void on_disconnected();
        };

        VADONEDITORCOMMON_API Client(asio::io_context& io_context, Observer& observer, LoggingInterface& logging_interface);

        VADONEDITORCOMMON_API void connect(const asio::ip::tcp::endpoint& endpoint);
        VADONEDITORCOMMON_API void send(const char* data, size_t size);
        VADONEDITORCOMMON_API void disconnect();

    private:
        void on_received(int connection_id, const char* data, size_t size) override;
        void on_connection_closed(int connection_id) override;

        asio::io_context& m_io_context;
        std::shared_ptr<Connection> m_connection;
        Observer& m_observer;
        LoggingInterface& m_logging_interface;
    };
}
#endif