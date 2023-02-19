#include <iostream>
#include <list>
#include <utility>
#include <tuple>
#include <sstream>
#include <map>
#include <mutex>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>

#include "ConvertPacket.hpp"
#include "P2P-packet.hpp"
#include "sha1.hpp"

#pragma once

class FssServer;

namespace P2P
{
    namespace fs = boost::filesystem;
    class UploadManager
    {
    public:
        struct UploadInfo
        {
            std::string path;
            size_t lastKnownLength;
            UploadInfo(std::string path, size_t lastKnownLength);
        };
        using clock = boost::chrono::system_clock;
        static const int max_buffer = 5000;
        static const int max_undelivered_packets = 50;
        static const int packet_timeout_seconds = 3;

    private:
        bool m_endpointset = false;
        boost::asio::io_service m_ioservice;
        socket m_socket;
        endpoint m_peerEndPoint;
        std::queue<UploadInfo> m_info;
        boost::thread m_uploadthread;
        bool m_running = false;

        std::mutex mapmutex;
        std::map<int, DownloadDataPacket> m_id_packetmap;
        std::map<int, clock::time_point> m_id_timeoutmap;
        int undelivered_packets;

        void ResendTimedout();
        void AddPacketEntry(fs::ifstream &istream, int packetOrderNumber, int amount, std::string fileHash);
        void UploadFile();
        void Upload();

    public:
        UploadManager();
        UploadManager(const UploadManager &uploadManager);
        void AddToQueue(std::vector<UploadInfo> files);

        // async

        void RemoveReceived(int packetOrderNumber);
        void SetEndPoint(endpoint endpoint_);
        void SendOpenConnection(int clientId, int peerId, endpoint serverEndPoint);
    };
}