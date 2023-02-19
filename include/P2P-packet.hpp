#include <iostream>
#include <list>
#include <utility>

#include <boost/asio.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>

#include "ConvertPacket.hpp"

#pragma once
namespace P2P
{
    using endpoint = boost::asio::ip::udp::endpoint;
    using socket = boost::asio::ip::udp::socket;
    using boost::asio::ip::udp;
    struct EndpointInfo
    {
        int endpointOwnerId;
        std::string hostname;
        int port;
        EndpointInfo();
        EndpointInfo(int endpointOwnderId, endpoint endpoint_);
        std::string ToString();
        void Print();
        endpoint ToEndpoint();
        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &endpointOwnerId;
            ar &hostname;
            ar &port;
        }
    };
    // sent from peer to mediator
    struct OpenConnectionPacket
    {
        int senderId;
        int peerId;
        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &senderId;
            ar &peerId;
        }
    };
    // send from peer to peer
    struct DownloadDataHeader
    {
        int packetOrderNumber;
        std::string sha1Check;
        std::string fileHash;
        int dataSize;
        DownloadDataHeader();
        DownloadDataHeader(int packetOrderNumber, std::string sha1Check, int dataSize, std::string fileHash);
        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &packetOrderNumber;
            ar &sha1Check;
            ar &dataSize;
            ar &fileHash;
        }
        void Print();
    };
    struct DownloadDataPacket
    {
        std::string headerString;
        std::string dataString;

        std::vector<boost::asio::mutable_buffers_1> buffers;
        DownloadDataPacket();
        DownloadDataPacket(std::string headerString, std::string dataString);
    };
}