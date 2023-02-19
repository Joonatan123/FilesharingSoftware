#include "upload-manager.hpp"
P2P::UploadManager::UploadInfo::UploadInfo(std::string path, size_t lastKnownLength) : path(path), lastKnownLength(lastKnownLength) {}

void P2P::UploadManager::ResendTimedout()
{
    // std::cout << "resend" << undelivered_packets << std::endl;
    auto now = clock::now();
    for (auto it = m_id_timeoutmap.begin(); it != m_id_timeoutmap.end(); it++)
    {
        // std::cout << (now > it->second);
        if (now > it->second)
        {
            // std::cout << "sending udp packet to: " << m_peerEndPoint.port() << std::endl;
            m_socket.send_to(m_id_packetmap[it->first].buffers, m_peerEndPoint);
            m_id_timeoutmap[it->first] = clock::duration(boost::chrono::seconds(packet_timeout_seconds)) + now;
        }
    }
}
void P2P::UploadManager::AddPacketEntry(fs::ifstream &istream, int packetOrderNumber, int amount, std::string fileHash)
{
    LOG_TRACE(p2p) << "adding entry and sending\n";
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(amount);
    istream.read(&buffer[0], amount);
    std::string dataString(&buffer[0], amount);
    std::string dataHash = sha1_8byte(dataString);

    DownloadDataHeader packetHeader(packetOrderNumber, dataHash, amount, fileHash);

    m_id_packetmap.insert(std::pair<int, DownloadDataPacket>(packetOrderNumber, DownloadDataPacket(ConvertMessageToString(packetHeader), dataString)));
    undelivered_packets += 1;
    std::vector<boost::asio::mutable_buffers_1> asd;

    m_socket.send_to(m_id_packetmap[packetOrderNumber].buffers, m_peerEndPoint);

    auto timeout = clock::duration(boost::chrono::seconds(packet_timeout_seconds)) + clock::now();

    m_id_timeoutmap.insert(std::pair<int, clock::time_point>(packetOrderNumber, timeout));
    // std::cout << "sent\n";
}
void P2P::UploadManager::UploadFile()
{
    UploadInfo info = m_info.front();
    m_info.pop();
    std::string path = info.path;

    std::string fileHash = sha1_8byte(path);
    int fileSize;
    try
    {
        fileSize = fs::file_size(path);
        LOG_INFO(p2p) << Log::Important() << "uploading file: " << path;
        LOG_INFO(p2p) << "uploading file: " << path << "    of size: " << fileSize;
    }
    catch (std::exception e)
    {
        LOG_INFO(p2p) << "uploading file: " << path;
        LOG_ERROR(p2p) << "invalid path for file size: " << path;
        throw std::exception();
    }
    int bytesLeft = fileSize;

    if (fileSize != info.lastKnownLength)
    {
        LOG_ERROR(p2p) << "filesize changed";
        throw std::exception();
    }
    fs::path p(path);
    fs::ifstream istream(p);
    int packetOrderNumber = 0;
    undelivered_packets = 0;
    while (bytesLeft > 0 || undelivered_packets > 0)
    {
        mapmutex.lock();
        for (; bytesLeft > 0 && undelivered_packets < max_undelivered_packets; packetOrderNumber++)
        {
            if (max_buffer < bytesLeft)
            {
                AddPacketEntry(istream, packetOrderNumber, max_buffer, fileHash);
                bytesLeft -= max_buffer;
            }
            else // if (bytesLeft > 0)
            {
                AddPacketEntry(istream, packetOrderNumber, bytesLeft, fileHash);
                bytesLeft -= bytesLeft;
            }
        }
        mapmutex.unlock();
        if (undelivered_packets == max_undelivered_packets)
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
        }
        mapmutex.lock();
        ResendTimedout();
        mapmutex.unlock();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    LOG_INFO(p2p) << Log::Important(true) << "file uploaded: " << path;
}
void P2P::UploadManager::Upload()
{
    m_running = true;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    while (m_info.size() != 0)
    {
        UploadFile();
    }
    m_running = false;
}

P2P::UploadManager::UploadManager() : m_socket(m_ioservice)
{
    m_socket.open(udp::v4());
}
P2P::UploadManager::UploadManager(const UploadManager &uploadManager) : m_socket(m_ioservice)
{
    m_socket.open(udp::v4());
}
void P2P::UploadManager::AddToQueue(std::vector<UploadInfo> files)
{
    for (UploadInfo file : files)
        m_info.push(file);
}

// async

void P2P::UploadManager::RemoveReceived(int packetOrderNumber)
{
    mapmutex.lock();
    if (m_id_packetmap.count(packetOrderNumber) == 1)
    {
        undelivered_packets -= 1;
        m_id_packetmap.erase(packetOrderNumber);
        m_id_timeoutmap.erase(packetOrderNumber);
    }
    mapmutex.unlock();
}
void P2P::UploadManager::SetEndPoint(endpoint endpoint_)
{
    m_endpointset = true;
    if (!m_running)
    {
        m_peerEndPoint = endpoint_;
        auto upload_function = boost::bind(&UploadManager::Upload, this);
        m_uploadthread = boost::thread{upload_function};
    }
}
void P2P::UploadManager::SendOpenConnection(int clientId, int peerId, endpoint serverEndPoint)
{
    OpenConnectionPacket packet;
    packet.peerId = peerId;
    packet.senderId = clientId;
    std::string packetString = ConvertMessageToString(packet);
    m_socket.send_to(boost::asio::buffer(packetString), serverEndPoint);
}
