#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <map>

#include <boost/serialization/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>

using std::string;

#include "file-list.hpp"

class ClientSystem
{
public:
    std::map<std::string, FilesystemEntry> m_hash_entrymap;
    std::map<std::string, std::string> m_hash_pathmap;

    std::vector<std::vector<FilesystemEntry>> folders;
    std::string client_name;
    unsigned long id = -1;
    bool online = true;

    bool FindEntryFromHash(FilesystemEntry &result, std::string hash);

    // only for use on the owned system
    bool FindPathFromHash(std::string &result, std::string hash);
    // only for use on the owned system
    bool FindPathFromHash(FilesystemEntry &result, std::string hash);
    // only for use on the owned system
    void MakeFolders(std::map<std::string, bool> &base_folder_names);
    void Print(bool full = false);
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &folders;
        ar &client_name;
        ar &online;
        ar &id;
    }

    ClientSystem();
    ClientSystem(std::string name);
    ClientSystem(unsigned long id);
};
void ReadCommand(std::vector<std::string> *cmd_buffer);
