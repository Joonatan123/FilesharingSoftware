#include <iostream>
#include <list>
#include <utility>
#include <tuple>
#include <sstream>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>

#include "ConvertPacket.hpp"
#include "P2P-packet.hpp"
#include "sha1.hpp"
#include "upload-manager.hpp"

#pragma once

class FssClient;

namespace P2P
{
    namespace fs = boost::filesystem;
    class DownloadManager
    {
    public:
        using callback_t = boost::_bi::bind_t<boost::_bi::unspecified, boost::_mfi::mf2<void, FssClient, int, int>, boost::_bi::list3<boost::_bi::value<FssClient *>, boost::arg<1>, boost::arg<2>>>;

        struct DownloadInfo
        {
            std::string downloadFolder;
            std::string name;
            std::string hash;
            size_t size;
            DownloadInfo(std::string downloadFolder, std::string name, std::string hash, size_t size);
        };

    private:
        using buffer_t = boost::array<char, UploadManager::max_buffer + 100>;
        using buffer_handle_t = std::list<buffer_t>::iterator;

        int m_peerid;
        int m_id;
        boost::asio::io_service m_ioservice;
        socket m_socket;

        bool m_running;
        std::queue<DownloadInfo> m_infoqueue;
        std::list<buffer_t> m_buffers;
        std::map<int, std::string> m_id_datamap;
        std::map<int, bool> m_id_receivedmap;
        boost::thread m_downloadthread;
        callback_t m_callback;

        std::map<int, std::string> m_id_packetmap;
        void Reset();
        void HandlePacket(buffer_handle_t bufferHandle, int &fileLeft, std::string fileHash, boost::system::error_code const &err, size_t receivedBytes);
        void DoPacketHandle(int &fileLeft, std::string fileHash);
        void FlushSocketBuffer();
        int BuildFile(fs::ofstream &ostream, int lastInsertId);
        void DownloadFile();
        void Download();

    public:
        // async

        void AddToQueue(std::vector<DownloadInfo> infoVec);
        DownloadManager(callback_t callback, int id, int peerId);
        DownloadManager(const DownloadManager &downloadManager);
        void SendOpenConnection(int clientId, int peerId, endpoint serverEndPoint);
    };
}