#include <iostream>
#include <list>
#include <utility>

#include <boost/asio.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>

#include "ConvertPacket.hpp"
#include "P2P-packet.hpp"
#include "upload-manager.hpp"
#include "download-manager.hpp"
#include "file-list.hpp"

namespace P2P
{
    class P2PClient
    {
        std::list<DownloadManager> m_downloadlist;
        std::map<int, UploadManager> m_peerid_upload; //
        std::map<int, std::list<DownloadManager>::iterator> m_peerid_download;

        DownloadManager::callback_t m_downloadcallback;

        endpoint m_serverEndPoint;
        int m_id = 0;

        void CreateDownloadManager(int peerId);

        void CreateUploadManager(int peerId);

    public:
        P2PClient(std::string hostname, unsigned short port, DownloadManager::callback_t downloadCallback);
        void SetId(int id);
        void AddToUploadQue(int peerId, std::vector<std::string> paths, std::vector<FilesystemEntry> entries);
        void AddToDownloadQue(int peerId, std::string downloadFolder, std::vector<FilesystemEntry> files);
        void SetEndpoint(int peerId, EndpointInfo info);
        void SetDownloadAck(int peerId, int packetOrderId);
    };
}