#include "P2P-packet.hpp"

P2P::EndpointInfo::EndpointInfo() {}
P2P::EndpointInfo::EndpointInfo(int endpointOwnderId, endpoint endpoint_)
{
    this->endpointOwnerId = endpointOwnderId;
    hostname = endpoint_.address().to_string();
    port = endpoint_.port();
}
std::string P2P::EndpointInfo::ToString()
{
    return hostname + ' ' + std::to_string(port);
}
void P2P::EndpointInfo::Print()
{
    std::cout << hostname << ' ' << port << std::endl;
}
P2P::endpoint P2P::EndpointInfo::ToEndpoint()
{
    return endpoint(boost::asio::ip::address::from_string(hostname), port);
}
/*template <class Archive>
void P2P::EndpointInfo::serialize(Archive &ar, const unsigned int version)
{
    ar &endpointOwnerId;
    ar &hostname;
    ar &port;
}*/
/*template <class Archive>
void P2P::OpenConnectionPacket::serialize(Archive &ar, const unsigned int version)
{
    ar &senderId;
    ar &peerId;
}*/

P2P::DownloadDataHeader::DownloadDataHeader() {}
P2P::DownloadDataHeader::DownloadDataHeader(int packetOrderNumber, std::string sha1Check, int dataSize, std::string fileHash) : packetOrderNumber(packetOrderNumber), sha1Check(sha1Check), dataSize(dataSize), fileHash(fileHash) {}
/*template <class Archive>
void P2P::DownloadDataHeader::serialize(Archive &ar, const unsigned int version)
{
    ar &packetOrderNumber;
    ar &sha1Check;
    ar &dataSize;
    ar &fileHash;
}*/
void P2P::DownloadDataHeader::Print()
{
    std::cout << "id: " << packetOrderNumber << " hash: " << sha1Check << " data size: " << dataSize << "fileHash: " << fileHash << std::endl;
}

P2P::DownloadDataPacket::DownloadDataPacket() {}
P2P::DownloadDataPacket::DownloadDataPacket(std::string headerString, std::string dataString) : headerString(headerString), dataString(dataString), buffers{boost::asio::buffer(this->headerString), boost::asio::buffer(this->dataString)}
{
}
