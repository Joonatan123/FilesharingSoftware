#include "file-list.hpp"
FilesystemEntry::FilesystemEntry() {}
FilesystemEntry::FilesystemEntry(int id, int parent_directory, int depth, std::string name, bool isDirectory, std::string hash, size_t size, std::time_t last_modified)
    : id(id), depth(depth), parent_directory(parent_directory), isDirectory(isDirectory), hash(hash), size(size), last_modified(last_modified)
{
    this->name = name;
}
std::string FilesystemEntry::MakeFileSizeString()
{
    std::vector<std::string> sizeStrings{"B", "KB", "MB", "GB", "TB"};
    double size = this->size;
    int factor = 0;
    for (; size >= 1000; factor++, size /= 1000)
    {
    }
    std::stringstream stream;
    stream << std::setprecision(4) << size << " " << sizeStrings[factor];
    double output = std::round(size);

    return stream.str();
}
std::string FilesystemEntry::MakeTimeString()
{
    std::stringstream stream;
    stream.str("");

    // Use a facet to display time in a custom format (only hour and minutes).
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet();
    facet->format("%d.%m.%Y %H:%M");
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << boost::posix_time::from_time_t(last_modified);
    return stream.str();
}
void FilesystemEntry::Print()
{
    for (int i = 0; i < depth; i++)
        std::cout << "  ";
    std::cout << name << " " << id << " " << parent_directory << " " << (isDirectory ? "true " : "false ")
              << (isDirectory ? "" : MakeFileSizeString()) << " " << hash << std::endl;
}
/*template <class Archive>
void FilesystemEntry::serialize(Archive &ar, const unsigned int version)
{
    ar &name;
    ar &parent_directory;
    ar &id;
    ar &depth;
    ar &isDirectory;
    ar &size;
    ar &hash;
    ar &last_modified;
}*/

void AddToMap(std::map<std::string, FilesystemEntry> &map, std::string str1, FilesystemEntry str2)
{
    if (map.count(str1) != 1)
    {
        // std::cout << "inserting to map: " << str1 << ' ' << str2 << std::endl;
        map.insert_or_assign(str1, str2);
    }
}
void AddToMap(std::map<std::string, std::string> &map, std::string str1, std::string str2)
{
    if (map.count(str1) != 1)
    {
        // std::cout << "inserting to map: " << str1 << ' ' << str2 << std::endl;
        map.insert_or_assign(str1, str2);
    }
}

void AddDirectory(fs::path p, std::vector<FilesystemEntry> &vec, std::map<std::string, FilesystemEntry> &entryMap, std::map<std::string, std::string> &pathMap, int parent, int depth)
{
    int id = vec.size();
    FilesystemEntry folder(id, parent, depth, p.filename().string(), true, sha1_8byte(p.string()));
    AddToMap(entryMap, sha1_8byte(p.string()), folder);
    AddToMap(pathMap, sha1_8byte(p.string()), p.string());
    vec.push_back(folder); // filename().string().c_str()
    fs::directory_iterator i(p);
    for (fs::directory_entry e : i)
    {
        if (fs::is_directory(e))
        {
            AddDirectory(e.path(), vec, entryMap, pathMap, id, depth + 1);
        }
        else if (fs::is_regular_file(e))
        {
            vec.push_back(FilesystemEntry(vec.size(), id, depth + 1, e.path().filename().string(), false, sha1_8byte(e.path().string()), boost::filesystem::file_size(e), boost::filesystem::last_write_time(e)));
            AddToMap(entryMap, sha1_8byte(e.path().string()), vec[vec.size() - 1]);
            AddToMap(pathMap, sha1_8byte(e.path().string()), e.path().string());
        }
    }
}

std::vector<FilesystemEntry> GetFilesFromPath(std::string input_path, std::map<std::string, FilesystemEntry> &entryMap, std::map<std::string, std::string> &pathMap)
{
    fs::path p(input_path);
    p = p.remove_leaf();
    std::vector<FilesystemEntry> &vec(*new std::vector<FilesystemEntry>);

    AddDirectory(p, vec, entryMap, pathMap);

    for (FilesystemEntry e : vec)
    {
        // e.Print();
    }

    return vec;
}