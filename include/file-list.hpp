#pragma once

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <string>
#include <vector>
#include <sstream>

#include "sha1.hpp"
#include "network.hpp"

namespace fs = boost::filesystem;

class FilesystemEntry
{
public:
    std::string name = "0";
    int parent_directory = -2;
    int id = -1;
    int depth = -1;
    bool isDirectory = false;
    size_t size = -1;
    std::string hash = "NULL";
    std::time_t last_modified = 0;
    FilesystemEntry();
    FilesystemEntry(int id, int parent_directory, int depth, std::string name, bool isDirectory, std::string hash, size_t size = -1, std::time_t last_modified = 0);
    std::string MakeFileSizeString();
    std::string MakeTimeString();
    void Print();
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &name;
        ar &parent_directory;
        ar &id;
        ar &depth;
        ar &isDirectory;
        ar &size;
        ar &hash;
        ar &last_modified;
    }
};

void AddToMap(std::map<std::string, FilesystemEntry> &map, std::string str1, FilesystemEntry str2);
void AddToMap(std::map<std::string, std::string> &map, std::string str1, std::string str2);

void AddDirectory(fs::path p, std::vector<FilesystemEntry> &vec, std::map<std::string, FilesystemEntry> &entryMap, std::map<std::string, std::string> &pathMap, int parent = -1, int depth = 0);

std::vector<FilesystemEntry> GetFilesFromPath(std::string input_path, std::map<std::string, FilesystemEntry> &entryMap, std::map<std::string, std::string> &pathMap);