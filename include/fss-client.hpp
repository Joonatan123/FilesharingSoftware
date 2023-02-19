#include <iostream>

#include "network-client.hpp"
#include "fss-class.hpp"
#include "P2P-client.hpp"

class FssClient
{
    P2P::P2PClient m_P2PClient;
    NetworkClient m_network;

    std::map<unsigned long, ClientSystem> m_id_peermap;

    std::map<std::string, bool> m_foldernames;

private:
    // ClientSystem m_id_peermap[0];
    boost::function<void(int, ClientSystem, bool)> fileUpdateCallback;
    std::vector<std::string> m_cmdbuffer;
    unsigned long m_id = 3;

    std::list<std::pair<boost::thread, std::string>> m_threads;

    void UpdateOrAddClientMap(unsigned long id, ClientSystem peer);
    void DownloadFiles(std::vector<std::string> hashes, int peerId);
    void LoadState(boost::archive::text_iarchive &ia);
    void UpdateServer();
public:
    FssClient(boost::function<void(int, ClientSystem, bool)> fileUpdateCallback);
    NetworkClient::callback_t make_callback();

    P2P::DownloadManager::callback_t make_download_callback();
    void send_download_ack(int peerId, int downloadPacketId);
    void SetName(std::string name);
    void RemoveFolder(std::string folder);
    void SaveState(boost::archive::text_oarchive &oa);
    void DownloadFiles(std::vector<FilesystemEntry> files, int peerId, std::string downloadFolder);
    void MakeFolders(std::vector<std::string> newFolders = {});
    void handle_packet(Header header, PacketBinary *packet, boost::system::error_code const &err);
    bool connect(std::string hostname, unsigned short port);
    void start(bool found, boost::archive::text_iarchive &ia);
};