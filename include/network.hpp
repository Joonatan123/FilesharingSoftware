#pragma once

#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include <iostream>
#include <assert.h>
#include <streambuf>
#include <mutex>

using namespace boost::asio;
using ip::tcp;
namespace asd = boost::asio;
namespace ip = boost::asio::ip;

struct Header
{
    enum MessageType
    {
        undefined = 0,
        FILES_LIST_PACKET = 1,
        FILE_DATA_PACKET,
        FILES_LIST_REQUEST,
        FILES_LIST_REFRESH,
        OPEN_CONNECTION,
        CLOSE_CONNECTION,
        ID_NOTIFICATION,
        PING_PONG,
        REDIRECT,
        REDIRECT_FAILURE,
        UDP_ADDRESS,
        DOWNLOAD_REQUEST,
        DOWNLOAD_INFO,
        DOWNLOAD_ACK
    };
    MessageType message_type = MessageType::undefined;

    // unsigned long sender = 0;

    static std::string TypeToString(MessageType type);
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &message_type;
    }
    Header(MessageType message_type);
    Header();
};
BOOST_CLASS_IMPLEMENTATION(Header, boost::serialization::object_serializable);
// packets do no own their data
// when receiving freeing is done at end of handle function
using PacketBinary = boost::archive::text_iarchive;
/*struct Packet
{
    Header header;
    // packets do no own their data
    // when receiving freeing is done at end of handle function
    const void *data;
    Packet(Header header, const void *data) : header(header), data(data) {}
};*/
struct Connection
{
    boost::asio::ip::tcp::socket socket;
    boost::asio::streambuf read_buffer;
    std::string name;
    void clear_read_buffer();
    Connection(boost::asio::io_service &io_service);
    Connection(boost::asio::io_service &io_service, size_t max_buffer_size);
};
namespace Network
{
    void CheckPacketValidity(std::string packet);
    template <class T, class U>
    void send_packet(Connection &connection, Header header, T message, U handler)
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
    void send_packet(Connection &connection, Header header, T message1, U message2, V handler)
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
    void send_packet(Connection &connection, Header header, T message1, U message2, V message3, W handler)
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
    // this funtion adds the \n at the end
    void send_string(Connection &connection, std::string message, T handler)
    {
        CheckPacketValidity(message + '\n');
        boost::asio::async_write(connection.socket, boost::asio::buffer(message + '\n'), handler);
    }

}