#include "network-client.hpp"

NetworkClient::NetworkClient(callback_t callback) : m_ioservice(), m_acceptor(m_ioservice), m_connection(m_ioservice), m_callback(callback) {}
void NetworkClient::clear_read_buffer()
{
    m_connection.read_buffer.consume(m_connection.read_buffer.size());
}
void NetworkClient::handle_read(boost::system::error_code const &err, size_t bytes_transfered)
{
    std::string lastPacket;
    if (err)
    {
        m_callback(Header(), nullptr, err);
        LOG_ERROR(network) << "erron on read handle: " << err.message();
        if (err == boost::asio::error::eof)
            m_connection.socket.close();
        return;
    }
    if (bytes_transfered > 0)
    {
        // std::string archive_header_data(std::string(std::istreambuf_iterator<char>(&m_connection.read_buffer), std::istreambuf_iterator<char>()));

        std::string archive_header_data;
        std::copy_n(boost::asio::buffers_begin(m_connection.read_buffer.data()), bytes_transfered, back_inserter(archive_header_data));
        m_connection.read_buffer.consume(bytes_transfered);

        lastPacket = archive_header_data;
        std::istringstream archive_header_stream(archive_header_data);

        Header header;
        std::string data_string;
        {
            boost::archive::text_iarchive archive_header(archive_header_stream, boost::archive::archive_flags::no_header);

            try
            {
                archive_header >> header;
                // LOG_ADD_THREAD_TAG(Log::Source::global);
                // LOG_WARNING << "warning";
                LOG_PACKET << Header::TypeToString(header.message_type);
            }
            catch (std::exception const &e)
            {
                LOG_ERROR(network) << "ERROR: invalid header\n"
                                   << lastPacket << std::endl
                                   << "bytes transferred: " << bytes_transfered << std::endl;
                throw(e);
            }
            m_callback(header, &archive_header, err);
        }
    }

    if (!err)
    {
        do_async_read();
    }
    else
    {
        // do_async_read();
    }
}
// must be called after the old packet has been handled
void NetworkClient::do_async_read()
{
    auto handler = boost::bind(&NetworkClient::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
    // boost::asio::async_read(m_connection.socket, m_connection.read_buffer, handler); // sizeof(Header)
    boost::asio::async_read_until(m_connection.socket, m_connection.read_buffer, "\n", handler);
}

void NetworkClient::handle_write(boost::system::error_code const &err)
{
    if (!err)
    {
        if (m_connection.socket.is_open())
        {
            // Write completed successfully and connection is open
        }
    }
    else
    {
        LOG_ERROR(network) << "write error: " << err.message();
    }
}
/*template <typename T>
void NetworkClient::send_packet(Header header, T message)
{
    if (!isConnected)
    {
        LOG_ERROR(network) << "cannot send packet: not connected";
        return;
    }
    auto handler = boost::bind(&NetworkClient::handle_write, this, boost::asio::placeholders::error);
    Network::send_packet(m_connection, header, message, handler);
}
template <typename T>
void NetworkClient::send_packet_redirect(int targetId, Header header, T message)
{
    if (targetId == m_id)
    {
        LOG_WARNING(network) << "cannot send redirect packet to self";
        return;
    }
    std::string packet;
    std::stringstream data;
    {
        boost::archive::text_oarchive oa_data(data, boost::archive::archive_flags::no_header);
        oa_data << header;
        oa_data << message;
        packet = data.str(); // this is to avoid \n at end of line causing server problems
        if (packet.find('\n') != -1)
            LOG_WARNING(network) << "sending data with \\n";
    }
    if (!isConnected)
    {
        LOG_WARNING(network) << "cannot send packet: not connected";
        return;
    }
    auto handler = boost::bind(&NetworkClient::handle_write, this, boost::asio::placeholders::error);
    Network::send_packet(m_connection, Header(Header::REDIRECT), targetId, packet, handler);
}
template <typename T, typename U>
void NetworkClient::send_packet_redirect(int targetId, Header header, T message1, U message2)
{
    if (targetId == m_id)
    {
        LOG_WARNING(network) << "cannot send redirect packet to self";
        return;
    }
    std::string packet;
    std::stringstream data;
    {
        boost::archive::text_oarchive oa_data(data, boost::archive::archive_flags::no_header);
        oa_data << header;
        oa_data << message1;
        oa_data << message2;
        packet = data.str(); // this is to avoid \n at end of line causing server problems
        if (packet.find('\n') != -1)
            LOG_WARNING(network) << "Warning: sending data with \\n";
    }
    if (!isConnected)
    {
        LOG_ERROR(network) << "cannot send packet: not connected";
        return;
    }
    auto handler = boost::bind(&NetworkClient::handle_write, this, boost::asio::placeholders::error);
    Network::send_packet(m_connection, Header(Header::REDIRECT), targetId, packet, handler);
}*/
// don't call. use FssClient::Connect instead
bool NetworkClient::connect(std::string server_name, unsigned short port)
{
    try
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(server_name), port);
        m_connection.socket.connect(endpoint);

        isConnected = true;
        return true;
    }
    catch (boost::system::system_error const &e)
    {
        LOG_ERROR(network) << Log::Important(true) << "could not connect: " << e.what();
        return false;
    }
}
void NetworkClient::run()
{
    do_async_read();

    m_ioservice.run();
}