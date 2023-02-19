#include "P2P-client.hpp"

void P2P::P2PClient::CreateDownloadManager(int peerId)
{
    if (m_id == 0)
    {
        std::cout << "no id received\n";
        return;
    }
    m_peerid_download.emplace(std::pair<int, std::list<DownloadManager>::iterator>(peerId, m_downloadlist.emplace(m_downloadlist.begin(), m_downloadcallback, m_id, peerId)));
}

void P2P::P2PClient::CreateUploadManager(int peerId)
{
    if (m_id == 0)
    {
        std::cout << "no id received\n";
        return;
    }
    m_peerid_upload.emplace(std::pair<int, UploadManager>(peerId, UploadManager()));
}

P2P::P2PClient::P2PClient(std::string hostname, unsigned short port, DownloadManager::callback_t downloadCallback) : m_downloadcallback(downloadCallback)
{
    m_serverEndPoint = endpoint(boost::asio::ip::address::from_string(hostname), port);
}
void P2P::P2PClient::SetId(int id)
{
    m_id = id;
}
void P2P::P2PClient::AddToUploadQue(int peerId, std::vector<std::string> paths, std::vector<FilesystemEntry> entries)
{
    if (m_peerid_upload.count(peerId) == 0)
        CreateUploadManager(peerId);
    m_peerid_upload[peerId].SendOpenConnection(m_id, peerId, m_serverEndPoint);
    std::vector<UploadManager::UploadInfo> info;
    for (int i = 0; i < paths.size(); i++)
        info.push_back(UploadManager::UploadInfo(paths[i], entries[i].size));
    m_peerid_upload[peerId].AddToQueue(info);
}
void P2P::P2PClient::AddToDownloadQue(int peerId, std::string downloadFolder, std::vector<FilesystemEntry> files)
{
    if (m_peerid_download.count(peerId) == 0)
        CreateDownloadManager(peerId);

    m_peerid_download[peerId]->SendOpenConnection(m_id, peerId, m_serverEndPoint);

    std::vector<DownloadManager::DownloadInfo> info;
    for (FilesystemEntry file : files)
        info.push_back(DownloadManager::DownloadInfo(downloadFolder, file.name, file.hash, file.size));

    m_peerid_download[peerId]->AddToQueue(info);
}
void P2P::P2PClient::SetEndpoint(int peerId, EndpointInfo info)
{
    if (m_peerid_upload.count(peerId) == 1)
    {
        m_peerid_upload[peerId].SetEndPoint(info.ToEndpoint());
    }
}
void P2P::P2PClient::SetDownloadAck(int peerId, int packetOrderId)
{
    m_peerid_upload[peerId].RemoveReceived(packetOrderId);
}