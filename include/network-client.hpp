#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "network.hpp"
#include "file-list.hpp"
#include "log.hpp"

#pragma once

class FssClient;

class NetworkClient
{
public:
    using callback_t = boost::_bi::bind_t<boost::_bi::unspecified, boost::_mfi::mf3<void, FssClient, Header, PacketBinary *, const boost::system::error_code &>, boost::_bi::list4<boost::_bi::value<FssClient *>, boost::arg<1>, boost::arg<2>, boost::arg<3>>>;
    callback_t m_callback;

private:
    boost::asio::io_service m_ioservice;
    boost::asio::ip::tcp::acceptor m_acceptor;
    Connection m_connection;

public:
    bool isConnected = false;
    int m_id;
    NetworkClient(callback_t callback);
    void clear_read_buffer();
    void handle_read(boost::system::error_code const &err, size_t bytes_transfered);
    // must be called after the old packet has been handled
    void do_async_read();

    void handle_write(boost::system::error_code const &err);
    template <typename T>
    void send_packet(Header header, T message)
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
    void send_packet_redirect(int targetId, Header header, T message)
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
    void send_packet_redirect(int targetId, Header header, T message1, U message2)
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
    }
    // don't call. use FssClient::Connect instead
    bool connect(std::string server_name, unsigned short port);
    void run();
};