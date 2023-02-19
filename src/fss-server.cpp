#include "fss-server.hpp"

unsigned long FssServer::add_client(NetworkServer::con_handle_t con_handle, unsigned long p_id)
{
    sys_handle_t sys_handle;
    unsigned long id = p_id;
    if (id == 0)
    {
        id = 1;
        while (m_id_clientmap.count(id) != 0)
            id++;

        LOG_INFO(main_app) << "inserting new ClientSystem with id: " << id;

        sys_handle = m_clients.emplace(m_clients.begin(), ClientSystem(id));
        m_id_clientmap.insert(std::pair<unsigned long, sys_handle_t>(id, sys_handle));
    }
    else
    {
        if (m_id_clientmap.count(id) != 0)
        {
            if (m_id_conmap.count(id) != 0)
            {
                LOG_WARNING(main_app) << "client claims entry, but is already in use";
                return add_client(con_handle);
            }
            LOG_INFO(main_app) << "client found with id";
            sys_handle = m_id_clientmap[id];
            m_id_clientmap[id]->online = true;
        }
        else
        {
            LOG_INFO(main_app) << "client claims entry should exist, but doesn't";
            return add_client(con_handle);
        }
    }
    m_id_conmap.insert(std::pair<unsigned long, NetworkServer::con_handle_t>(id, con_handle));
    m_con_clientmap.insert(std::pair<NetworkServer::con_handle_t, sys_handle_t>(con_handle, sys_handle));

    return id;
}
unsigned long FssServer::FindId(NetworkServer::con_handle_t con_handle)
{
    for (std::map<unsigned long, NetworkServer::con_handle_t>::iterator iter = m_id_conmap.begin(); iter != m_id_conmap.end(); ++iter)
    {
        if (iter->second == con_handle)
        {
            return iter->first;
        }
    }
    return -1;
}
// when a entry is updated send the update to all other clients
void FssServer::SendSingleUpdateToAll(unsigned long id_excluded)
{
    for (std::map<unsigned long, NetworkServer::con_handle_t>::iterator iter = m_id_conmap.begin(); iter != m_id_conmap.end(); ++iter)
    {
        if (iter->first != id_excluded)
        {
            m_network.send_packet(m_id_conmap[iter->first], Header(Header::FILES_LIST_PACKET), std::pair<unsigned long, ClientSystem>(id_excluded, *m_id_clientmap[id_excluded]));
        }
    }
}
void FssServer::send_full_peer_update(NetworkServer::con_handle_t con_handle, unsigned long id)
{
    for (std::map<unsigned long, sys_handle_t>::iterator iter = m_id_clientmap.begin(); iter != m_id_clientmap.end(); ++iter)
    {
        if (iter->first != id)
        {
            // boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
            m_network.send_packet(con_handle, Header(Header::FILES_LIST_PACKET), std::pair<unsigned long, ClientSystem>(iter->first, *iter->second));
        }
    }
}

void FssServer::SaveState()
{
    LOG_INFO(main_app) << "Saving state";
    std::ofstream ofs("server_state");

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << m_clients;
        // archive and stream closed when destructors are called
    }
}
void FssServer::LoadState()
{
    LOG_INFO(main_app) << "Loading state";
    if (m_clients.size() != 0)
    {
        LOG_INFO(main_app) << "can't load state: client entry already present";
        return;
    }
    std::ifstream ifs("server_state");

    {
        boost::archive::text_iarchive ia(ifs);
        // write class instance to archive
        ia >> m_clients;
        // archive and stream closed when destructors are called
    }
    for (sys_handle_t it = m_clients.begin(); it != m_clients.end(); it++)
    {
        it->online = false;
        m_id_clientmap.insert(std::pair<const unsigned long, FssServer::sys_handle_t>(it->id, it));
    }
}

FssServer::FssServer() : m_network(make_callback()), m_mediator(55555, make_mediator_callback()) {}
NetworkServer::callback_t FssServer::make_callback()
{
    auto temp = boost::bind(boost::mem_fn(&FssServer::handle_packet), this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4);
    return temp;
}
P2P::P2PMediator::callback_t FssServer::make_mediator_callback()
{
    auto temp = boost::bind(boost::mem_fn(&FssServer::handle_mediator_write), this, boost::placeholders::_1, boost::placeholders::_2);
    return temp;
}
void FssServer::handle_packet(Header header, PacketBinary *packet, NetworkServer::con_handle_t con_handle, boost::system::error_code const &err)
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
    // m_network.send_packet(con_handle, Header(Header::CLOSE_CONNECTION), 123);

    // m_network.send_packet(con_handle, Header(Header::FILES_LIST_REQUEST), (double)(123));
    // m_network.send_packet(con_handle, Header(Header::CLOSE_CONNECTION), (double)(123));

    // m_network.send_packet(con_handle, Header(Header::FILES_LIST_PACKET), FilesystemEntry(1,-1,1,"hello a", false));

    switch (header.message_type)
    {
    case Header::OPEN_CONNECTION:
    {
        std::pair<unsigned long, std::string> info;
        *packet >> info;

        unsigned long id = info.first;
        id = add_client(con_handle, id);
        m_id_clientmap[id]->client_name = info.second;

        m_network.send_packet(con_handle, Header(Header::ID_NOTIFICATION), id);

        send_full_peer_update(con_handle, id);

        SendSingleUpdateToAll(id);

        break;
    }
    case Header::CLOSE_CONNECTION:
    {
        unsigned long id = -1;
        for (auto &val : m_id_conmap)
        {
            if (val.second == con_handle)
            {
                id = val.first;
                break;
            }
        }
        m_id_clientmap[id]->online = false;
        m_id_conmap.erase(id);
        m_con_clientmap.erase(con_handle);
        LOG_INFO(main_app) << "erased connection";

        SendSingleUpdateToAll(id);
        break;
    }
    case Header::PING_PONG:
    {
        int id;
        *packet >> id;
        LOG_INFO(main_app) << "int received: " << id;
        if (id > 1)
            m_network.send_packet(con_handle, Header(Header::PING_PONG), id - 1);
        break;
    }
    case Header::FILES_LIST_PACKET:
    {
        ClientSystem system;
        unsigned long id = FindId(con_handle);
        *packet >> system;
        system.id = id;
        system.online = true;
        *m_con_clientmap[con_handle] = system;
        if (id != -1)
            SendSingleUpdateToAll(id);
        break;
    }
    case Header::REDIRECT:
    {
        int targetId;
        *packet >> targetId;
        std::string redirectPacket;
        *packet >> redirectPacket;
        if (m_id_conmap.count(targetId) == 1)
        {
            // std::cout << "redirecting: " << redirectPacket << std::endl;
            m_network.send_string(m_id_conmap[targetId], redirectPacket);
        }
        else
        {
            m_network.send_packet(con_handle, Header(Header::REDIRECT_FAILURE), 0);
        }
        break;
    }
    case Header::FILES_LIST_REFRESH:
    {
        for (std::map<unsigned long, NetworkServer::con_handle_t>::iterator iter = m_id_conmap.begin(); iter != m_id_conmap.end(); ++iter)
        {
            if (iter->first != m_con_clientmap[con_handle]->id)
            {
                m_network.send_packet(m_id_conmap[iter->first], Header(Header::FILES_LIST_REQUEST), 0);
            }
        }

        break;
    }
    default:
    {
        LOG_WARNING(main_app) << "unhandled packet";
    }
    }
}
void FssServer::handle_mediator_write(int receiverId, P2P::EndpointInfo endPointInfo)
{
    m_network.send_packet(m_id_conmap[receiverId], Header(Header::UDP_ADDRESS), endPointInfo);
}
void FssServer::start()
{
    m_network.listen(12345);
    auto temp = boost::bind(&NetworkServer::run, &m_network);
    boost::thread t{temp};
    auto temp2 = boost::bind(&ReadCommand, &m_cmdbuffer);
    boost::thread t2{temp2};
    auto temp3 = boost::bind(&P2P::P2PMediator::Run, &m_mediator);
    boost::thread t3{temp3};
    while (true)
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(150));

        for (int i = 0; i < m_cmdbuffer.size(); i++)
        {
            std::vector<std::string> cmd;

            boost::split(cmd, m_cmdbuffer[i], boost::is_any_of(" "));

            if (cmd[0] == "print")
            {
                bool full = false;
                if (cmd.size() > 1)
                    full = cmd[1] == "full";
                for (std::list<ClientSystem>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
                {
                    it->Print(full);
                }
            }
            else if (cmd[0] == "save")
            {
                SaveState();
            }
            else if (cmd[0] == "load")
            {
                LoadState();
            }
            else if (cmd[0] == "ping")
            {
                int amount = 10;
                if (cmd.size() > 1)
                    amount = std::stoi(cmd[1]);
                m_network.send_packet(m_id_conmap[1], Header(Header::PING_PONG), amount);
            }
            else
            {
                LOG_ERROR(main_app) << "unrecognized command: " << m_cmdbuffer[i];
            }
        }
        m_cmdbuffer.clear();
    }
}

int main()
{
    Log::init();
    std::cout << "working\n";
    FssServer server;
    server.start();
    for (int i = 0; true; i++)
        i++;
    return 0;
}