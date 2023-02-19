#include "fss-class.hpp"
bool ClientSystem::FindEntryFromHash(FilesystemEntry &result, std::string hash)
{
    for (std::vector<FilesystemEntry> vec : folders)
    {
        for (FilesystemEntry fs : vec)
        {
            if (fs.hash == hash)
            {
                result = fs;
                return true;
            }
        }
    }
    return false;
}

// only for use on the owned system
bool ClientSystem::FindPathFromHash(std::string &result, std::string hash)
{
    if (m_hash_pathmap.count(hash) == 1)
    {
        result = m_hash_pathmap[hash];
        return true;
    }
    else
    {
        return false;
    }
}
// only for use on the owned system
bool ClientSystem::FindPathFromHash(FilesystemEntry &result, std::string hash)
{
    if (m_hash_entrymap.count(hash) == 1)
    {
        result = m_hash_entrymap[hash];
        return true;
    }
    else
    {
        return false;
    }
}
// only for use on the owned system
void ClientSystem::MakeFolders(std::map<std::string, bool> &base_folder_names)
{
    folders.clear();
    m_hash_pathmap.clear();
    m_hash_entrymap.clear();
    for (std::pair<string, bool> path : base_folder_names)
        folders.push_back(GetFilesFromPath(path.first, m_hash_entrymap, m_hash_pathmap));
}
void ClientSystem::Print(bool full)
{
    std::cout << std::endl
              << "client name: " << client_name << "    id: " << id << std::endl;
    std::cout << "status: " << (online ? "online" : "offline") << std::endl;
    if (full)
    {
        for (std::vector<FilesystemEntry> v : folders)
        {
            std::cout << std::endl;
            for (FilesystemEntry fs : v)
            {
                fs.Print();
            }
        }
    }
    std::cout << std::endl;
}
/*template <class Archive>
void ClientSystem::serialize(Archive &ar, const unsigned int version)
{
    ar &folders;
    ar &client_name;
    ar &online;
    ar &id;
}*/

ClientSystem::ClientSystem() {}
ClientSystem::ClientSystem(std::string name) : client_name(name) {}
ClientSystem::ClientSystem(unsigned long id) : id(id) {}

void ReadCommand(std::vector<std::string> *cmd_buffer)
{
    while (true)
    { //?
        std::string cmd;
        std::getline(std::cin, cmd);
        cmd_buffer->push_back(cmd);
    }
}
