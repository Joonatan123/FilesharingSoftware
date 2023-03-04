#include "download-manager.hpp"
P2P::DownloadManager::DownloadInfo::DownloadInfo(std::string downloadFolder, std::string name, std::string hash, size_t size) : downloadFolder(downloadFolder), name(name), hash(hash), size(size) {}

void P2P::DownloadManager::Reset()
{
    int count = 0;
    for (auto it = m_id_receivedmap.begin(); it != m_id_receivedmap.end(); it++)
    {
        count++;
    }
    // std::cout << "receivemap COunt: " << count << std::endl;
    m_id_receivedmap.clear();

    count = 0;
    for (auto it = m_id_datamap.begin(); it != m_id_datamap.end(); it++)
    {
        count++;
    }
    // std::cout << "m_id_datamap COunt: " << count << std::endl;
    m_id_datamap.clear();

    /*count = 0;
    for(auto it = m_buffers.begin(); it != m_buffers.end(); it++){
        count ++;
    }
    std::cout << "m_id_datamap COunt: " << count << std::endl;
    m_buffers.clear();*/

    count = 0;
    for (auto it = m_id_packetmap.begin(); it != m_id_packetmap.end(); it++)
    {
        count++;
    }
    // std::cout << "m_id_packetmap COunt: " << count << std::endl;
    m_id_packetmap.clear();
}
void P2P::DownloadManager::HandlePacket(buffer_handle_t bufferHandle, int &fileLeft, std::string fileHash, boost::system::error_code const &err, size_t receivedBytes)
{
    LOG_TRACE(p2p) << "udp packet received of size: " << receivedBytes;
    DownloadDataHeader header;
    char *buffer = &((*bufferHandle)[0]);
    ConvertBufferToMessage(buffer, header);

    if (m_id_receivedmap.count(header.packetOrderNumber) != 0)
    {
        // packet already received
        LOG_TRACE(p2p) << "pakcet already received";
        m_buffers.erase(bufferHandle);
        DoPacketHandle(fileLeft, fileHash);
        return;
    }
    // header.Print();
    int endOfHeader = -1;
    for (int i = 0; i < 100; i++)
        if (buffer[i] == '\n')
        {
            endOfHeader = i + 1;
            break;
        }
    std::string dataString(&(buffer[endOfHeader]), header.dataSize);
    if (sha1_8byte(dataString) != header.sha1Check || header.fileHash != fileHash)
    {
        if (header.fileHash != fileHash)
            LOG_TRACE(p2p) << "invalid fileHash in received packet: " << header.fileHash << " != " << fileHash;
        // std::cout << "start#" << dataString << "#end";
        m_buffers.erase(bufferHandle);
        // std::cout << "wrong hash in received packet\n";
        DoPacketHandle(fileLeft, fileHash);
        return;
    }
    m_id_receivedmap.insert_or_assign(header.packetOrderNumber, true);
    m_callback(m_peerid, header.packetOrderNumber);
    fileLeft -= header.dataSize;
    m_id_datamap.emplace(std::pair<int, std::string>(header.packetOrderNumber, dataString));
    m_buffers.erase(bufferHandle);
    if (fileLeft > 0)
        DoPacketHandle(fileLeft, fileHash);
}
void P2P::DownloadManager::DoPacketHandle(int &fileLeft, std::string fileHash)
{
    auto bufferHandle = m_buffers.emplace(m_buffers.begin());
    auto handler = boost::bind(&DownloadManager::HandlePacket, this, bufferHandle, boost::ref(fileLeft), fileHash, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

    m_socket.async_receive(boost::asio::buffer(*bufferHandle, 5100), handler);
    LOG_TRACE(p2p) << "started packet handle, size: " << fileLeft;
}
void P2P::DownloadManager::FlushSocketBuffer()
{
    size_t amount = m_socket.available();
    std::vector<char> buf(amount);
    m_socket.receive(boost::asio::buffer(buf.data(), amount));
}
int P2P::DownloadManager::BuildFile(fs::ofstream &ostream, int lastInsertId)
{
    bool found = true;
    while (found)
    {
        if (m_id_datamap.count(lastInsertId + 1) >= 1)
        {
            found = true;
            ostream << m_id_datamap[lastInsertId + 1];
            m_id_datamap.erase(lastInsertId + 1);
            lastInsertId += 1;
        }
        else
        {
            found = false;
        }
    }
    return lastInsertId;
}
void P2P::DownloadManager::DownloadFile()
{
    FlushSocketBuffer();
    Reset();
    DownloadInfo info = m_infoqueue.front();
    m_infoqueue.pop();
    std::string filename = info.name;
    int fileSize = info.size;

    fs::path p(info.downloadFolder);
    p /= fs::path(filename);
    LOG_INFO(p2p) << "downloading to: " << p.string();
    LOG_INFO(p2p) << Log::Important() << "downloading file: " << p.filename().string();
    fs::ofstream ostream(p, std::ios::binary);
    int lastWrittenPacketId = 0;

    int fileLeft = fileSize;

    DoPacketHandle(fileLeft, info.hash);
    m_ioservice.restart();

    int lastInsertId = -1;
    while (fileLeft != 0)
    {
        m_ioservice.run_one();
        // std::cout << "runone: " << m_ioservice.run_one();
        lastInsertId = BuildFile(ostream, lastInsertId);
        // std::cout << "left: " << fileLeft << std::endl;
    }
    // m_id_receivedmap.clear();
}
void P2P::DownloadManager::Download()
{
    m_running = true;
    LOG_INFO(p2p) << Log::Important(true) << "starting download";
    while (m_infoqueue.size() != 0)
    {
        DownloadFile();
    }
    LOG_INFO(p2p) << Log::Important(true) << "finished download";
    m_running = false;
}

// async

void P2P::DownloadManager::AddToQueue(std::vector<DownloadInfo> infoVec)
{
    for (DownloadInfo info : infoVec)
        m_infoqueue.push(info);

    if (!m_running)
    {
        auto upload_function = boost::bind(&DownloadManager::Download, this);
        m_downloadthread = boost::thread{upload_function};
    }
}
P2P::DownloadManager::DownloadManager(callback_t callback, int id, int peerId) : m_socket(m_ioservice), m_callback(callback)
{
    m_id = id;
    m_peerid = peerId;
    m_socket.open(udp::v4());
    LOG_TRACE(p2p) << "Opened udp listener";
    // DoPacketHandle(); // only gets run when run on io context executes
}
P2P::DownloadManager::DownloadManager(const DownloadManager &downloadManager) : m_socket(m_ioservice), m_callback(downloadManager.m_callback)
{

    m_socket.open(udp::v4());
    LOG_TRACE(p2p) << "Opened udp listener";
    m_id = downloadManager.m_id;
    m_peerid = downloadManager.m_peerid;
    // DoPacketHandle(); // only gets run when run on io context executes
}
void P2P::DownloadManager::SendOpenConnection(int clientId, int peerId, endpoint serverEndPoint)
{
    OpenConnectionPacket packet;
    packet.peerId = peerId;
    packet.senderId = clientId;
    std::string packetString = ConvertMessageToString(packet);
    m_socket.send_to(boost::asio::buffer(packetString), serverEndPoint);
}
