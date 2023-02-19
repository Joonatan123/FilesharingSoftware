#include "network.hpp"
std::string Header::TypeToString(MessageType type)
{
    switch (type)
    {
    case FILES_LIST_PACKET:
        return "FILES_LIST_PACKET";
    case FILE_DATA_PACKET:
        return "FILE_DATA_PACKET";
    case FILES_LIST_REQUEST:
        return "FILES_LIST_REQUEST";
    case OPEN_CONNECTION:
        return "OPEN_CONNECTION";
    case CLOSE_CONNECTION:
        return "CLOSE_CONNECTION";
    case FILES_LIST_REFRESH:
        return "FILES_LIST_REFRESH";
    case ID_NOTIFICATION:
        return "ID_NOTIFICATION";
    case PING_PONG:
        return "PING_PONG";
    case REDIRECT:
        return "REDIRECT";
    case REDIRECT_FAILURE:
        return "REDIRECT_FAILURE";
    case UDP_ADDRESS:
        return "UDP_ADDRESS";
    case DOWNLOAD_REQUEST:
        return "DOWNLOAD_REQUEST";
    case DOWNLOAD_INFO:
        return "DOWNLOAD_INFO";
    case DOWNLOAD_ACK:
        return "DOWNLOAD_ACK";

    default:
        return "undefined";
    }
}
/*template <class Archive>
void Header::serialize(Archive &ar, const unsigned int version)
{
    ar &message_type;
}*/
Header::Header(MessageType message_type) : message_type(message_type) {}
Header::Header() {}

void Connection::clear_read_buffer()
{
    read_buffer.consume(read_buffer.size());
}
Connection::Connection(boost::asio::io_service &io_service) : socket(io_service), read_buffer() {}
Connection::Connection(boost::asio::io_service &io_service, size_t max_buffer_size) : socket(io_service), read_buffer(max_buffer_size) {}

void Network::CheckPacketValidity(std::string packet)
{
    int count = 0;
    for (int i = 0; i < packet.size(); i++)
    {
        if (packet[i] == '\n')
            count++;
    }
    if (count != 1)
    {
        std::cout << "WARNING!!!\n\n\n\n\nINVALID AMOUNT OF \\n IN PACKET\n";
    }
}
/*template <class T, class U>
void Network::send_packet(Connection &connection, Header header, T message, U handler)
{
    std::stringstream data;
    {
        boost::archive::text_oarchive oa_data(data, boost::archive::archive_flags::no_header);
        oa_data << header;
        oa_data << message;
        if (data.str().find('\n') != -1)
            std::cout << "Warning: sending data with \\n\n";
    }
    CheckPacketValidity(data.str());
    boost::asio::async_write(connection.socket, boost::asio::buffer(data.str()), handler);
}
template <class T, class U, class V>
void Network::send_packet(Connection &connection, Header header, T message1, U message2, V handler)
{
    std::stringstream data;
    {
        boost::archive::text_oarchive oa_data(data, boost::archive::archive_flags::no_header);
        oa_data << header;
        oa_data << message1;
        oa_data << message2;

        if (data.str().find('\n') != -1)
            std::cout << "Warning: sending data with \\n\n";
    }
    CheckPacketValidity(data.str());
    boost::asio::async_write(connection.socket, boost::asio::buffer(data.str()), handler);
}
template <class T, class U, class V, class W>
void Network::send_packet(Connection &connection, Header header, T message1, U message2, V message3, W handler)
{
    std::stringstream data;
    {
        boost::archive::text_oarchive oa_data(data, boost::archive::archive_flags::no_header);
        oa_data << header;
        oa_data << message1;
        oa_data << message2;
        oa_data << message3;

        if (data.str().find('\n') != -1)
            std::cout << "Warning: sending data with \\n\n";
    }
    CheckPacketValidity(data.str());
    boost::asio::async_write(connection.socket, boost::asio::buffer(data.str()), handler);
}
template <class T>
// this funtion add the \n at the end
void Network::send_string(Connection &connection, std::string message, T handler)
{
    CheckPacketValidity(message + '\n');
    boost::asio::async_write(connection.socket, boost::asio::buffer(message + '\n'), handler);
}*/