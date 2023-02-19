#include "fss-client.hpp"

void FssClient::UpdateOrAddClientMap(unsigned long id, ClientSystem peer)
{
    m_id_peermap[id] = peer;
    fileUpdateCallback(id, peer, true);
}
void FssClient::DownloadFiles(std::vector<std::string> hashes, int peerId)
{
    std::vector<FilesystemEntry> fileEntries;
    for (std::string hash : hashes)
    {
        FilesystemEntry result;
        if (m_id_peermap[peerId].FindEntryFromHash(result, hash))
            fileEntries.push_back(result);
        result.Print();
    }
    DownloadFiles(fileEntries, peerId, "Download");
}
void FssClient::LoadState(boost::archive::text_iarchive &ia)
{
    LOG_INFO(main_app) << "Loading state";
    if (m_id_peermap.size() != 0)
    {
        LOG_WARNING(main_app) << "cannot load state: peer entry already exists";
        return;
    }

    // write class instance to archive
    // write class instance to archive
    ia >> m_id;
    ia >> m_foldernames;
    ia >> m_id_peermap;
    // archive and stream closed when destructors are called
    MakeFolders();
    for (auto it = m_id_peermap.begin(); it != m_id_peermap.end(); it++) // std::pair<unsigned long, ClientSystem> system : m_id_peermap
    {
        it->second.online = false;
        fileUpdateCallback(it->first, it->second, false);
    }
}
// send a copy of this system to server
void FssClient::UpdateServer()
{
    m_network.send_packet(Header(Header::FILES_LIST_PACKET), m_id_peermap[0]);
}
FssClient::FssClient(boost::function<void(int, ClientSystem, bool)> fileUpdateCallback) : fileUpdateCallback(fileUpdateCallback), m_network(make_callback()), m_P2PClient("127.0.0.1", 55555, make_download_callback()) {}
NetworkClient::callback_t FssClient::make_callback()
{
    auto temp = boost::bind(boost::mem_fn(&FssClient::handle_packet), this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3);
    return temp;
}

P2P::DownloadManager::callback_t FssClient::make_download_callback()
{
    auto temp = boost::bind(boost::mem_fn(&FssClient::send_download_ack), this, boost::placeholders::_1, boost::placeholders::_2);

    return temp;
}
void FssClient::send_download_ack(int peerId, int downloadPacketId)
{
    LOG_TRACE(p2p) << "sending downalod packet ACK: " << downloadPacketId;
    m_network.send_packet_redirect(peerId, Header(Header::DOWNLOAD_ACK), m_id, downloadPacketId);
}
void FssClient::SetName(std::string name)
{
    m_id_peermap[0].client_name = name;
    fileUpdateCallback(0, m_id_peermap[0], true);
    UpdateServer();
    LOG_INFO(main_app) << "changing name of this system to: " << name;
}
void FssClient::RemoveFolder(std::string folder)
{
    LOG_DEBUG(main_app) << folder;
    for (auto it = m_foldernames.begin(); it != m_foldernames.end(); it++)
    {
        if (folder == boost::filesystem::path(it->first).remove_leaf().filename().string())
        {
            m_foldernames.erase(it);
            LOG_DEBUG(main_app) << "found removed";
            break;
        }
    }
    MakeFolders();
}
void FssClient::SaveState(boost::archive::text_oarchive &oa)
{
    LOG_INFO(main_app) << "Saving state";

    // write class instance to archive
    oa << m_id;
    oa << m_foldernames;
    oa << m_id_peermap;
    // archive and stream closed when destructors are called
}
void FssClient::DownloadFiles(std::vector<FilesystemEntry> files, int peerId, std::string downloadFolder)
{
    std::vector<std::string> fileHashes;
    // std::vector<std::string> fileNames;
    for (FilesystemEntry entry : files)
    {
        fileHashes.push_back(entry.hash);
        // fileNames.push_back(entry.name);
    }
    m_network.send_packet_redirect(peerId, Header(Header::DOWNLOAD_REQUEST), m_id, fileHashes);
    m_P2PClient.AddToDownloadQue(peerId, downloadFolder, files);
}
void FssClient::MakeFolders(std::vector<std::string> newFolders)
{

    for (std::string folder : newFolders)
    {
        if (*folder.end() != '/')
            LOG_WARNING(file) << "adding folder path without \'/\' ending";
        m_foldernames[folder] = true;
        LOG_INFO(main_app) << folder << " added to folders";
    }
    m_id_peermap[0].MakeFolders(m_foldernames);
    fileUpdateCallback(0, m_id_peermap[0], false);
    UpdateServer();
}
void FssClient::handle_packet(Header header, PacketBinary *packet, boost::system::error_code const &err)
{
    if (err)
    {
        if (err == boost::asio::error::eof)
        {
            header.message_type = Header::CLOSE_CONNECTION;
        }
        else
        {
            LOG_ERROR(main_app) << "erron on packet handle: " << err.message();
            return;
        }
    }
    // std::cout << Header::TypeToString(header.message_type) << " type packet received" << std::endl;
    switch (header.message_type)
    {
    case Header::FILES_LIST_PACKET:
    {
        std::pair<unsigned long, ClientSystem> pair;
        *packet >> pair;

        UpdateOrAddClientMap(pair.first, pair.second);

        break;
    }
    case Header::FILES_LIST_REQUEST:
    {
        UpdateServer();
        break;
    }
    case Header::CLOSE_CONNECTION:
    {
        LOG_INFO(main_app) << Log::Important() << "connection lost";
        m_network.isConnected = false;
        /*int id;
        for (auto &)
        {
            if (it.second == K)
            {
                cout << it.first << ' ';
                a = false;
            }
        }
        if (id > 1)
            m_network.send_packet(Header(Header::CLOSE_CONNECTION), id - 1);*/
        break;
    }
    case Header::ID_NOTIFICATION:
    {
        LOG_INFO(main_app) << Log::Important() << "Connected";
        *packet >> m_id;
        m_P2PClient.SetId(m_id);
        m_network.m_id = m_id;
        LOG_INFO(main_app) << "id received: " << m_id;

        fileUpdateCallback(0, m_id_peermap[0], true);
        break;
    }
    case Header::UDP_ADDRESS:
    {
        P2P::EndpointInfo peerEndpointInfo;
        *packet >> peerEndpointInfo;
        LOG_TRACE(main_app) << peerEndpointInfo.ToString();
        m_P2PClient.SetEndpoint(peerEndpointInfo.endpointOwnerId, peerEndpointInfo);
        break;
    }
    case Header::DOWNLOAD_REQUEST:
    {
        int peerId;
        *packet >> peerId;
        std::vector<std::string> hashes;
        *packet >> hashes;
        std::vector<std::string> filePaths;
        std::vector<FilesystemEntry> fileEntries;
        for (std::string hash : hashes)
        {
            std::string filePath;
            if (m_id_peermap[0].FindPathFromHash(filePath, hash))
            {
                FilesystemEntry entry;
                m_id_peermap[0].FindEntryFromHash(entry, hash);
                LOG_INFO(main_app) << "download request from id: " << peerId << ' ' << filePath;
                filePaths.push_back(filePath);
                fileEntries.push_back(entry);
            }
            else
            {
                LOG_WARNING(main_app) << "requested download file could not be found";
            }
        }

        m_P2PClient.AddToUploadQue(peerId, filePaths, fileEntries);

        // m_network.send_packet_redirect(peerId, Header(Header::DOWNLOAD_INFO), m_id, fileInfo);

        break;
    }
    /*case Header::DOWNLOAD_INFO:
    {
        int peerId;
        *packet >> peerId;
        std::vector<FilesystemEntry> fileInfo;
        *packet >> fileInfo;

        m_P2PClient.SetDownloadInfo(peerId, fileInfo);

        break;
    }*/
    case Header::DOWNLOAD_ACK:
    {
        int peerId;
        *packet >> peerId;
        int packetOrderId;
        *packet >> packetOrderId;

        m_P2PClient.SetDownloadAck(peerId, packetOrderId);

        break;
    }
    case Header::PING_PONG:
    {
        int id;
        *packet >> id;
        LOG_INFO(main_app) << "int received: " << id;
        if (id > 1)
            m_network.send_packet(Header(Header::PING_PONG), id - 1);
        break;
    }
    default:
    {
        LOG_WARNING(main_app) << "unhandled packet";
        break;
    }
    }
}
bool FssClient::connect(std::string hostname, unsigned short port)
{
    if (m_network.connect(hostname, port))
    {
        auto run_function = boost::bind(&NetworkClient::run, &m_network);
        m_threads.push_back(std::pair<boost::thread, std::string>(boost::thread{run_function}, "run"));

        boost::this_thread::sleep_for(boost::chrono::milliseconds(200));

        m_network.send_packet(Header(Header::OPEN_CONNECTION), std::pair<unsigned long, std::string>(m_id, m_id_peermap[0].client_name));

        MakeFolders();

        fileUpdateCallback(0, m_id_peermap[0], false);

        UpdateServer();

        return true;
    }
    else
    {
        return false;
    }
}
void FssClient::start(bool found, boost::archive::text_iarchive &ia)
{
    if (found)
    {
        LoadState(ia);
    }
    else
    {
        m_id_peermap[0] = ClientSystem(0);
    }
}