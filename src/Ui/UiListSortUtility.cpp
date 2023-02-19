#include "UiListSortUtility.hpp"

#include <boost/algorithm/string.hpp>

// the interval from [s to m] and [m+1 to e] in v are sorted
// the function will merge both of these intervals
// such the interval from [s to e] in v becomes sorted

// true = in right order, false = in wrong order
bool CompareEntries(FilesystemEntry e1, FilesystemEntry e2, int direction, int item)
{
    bool output;
    if (e1.isDirectory && !e2.isDirectory)
        return true;

    if (!e1.isDirectory && e2.isDirectory)
        return false;

    if (e1.isDirectory && e2.isDirectory)
    {
        if (item != 0)
            return boost::algorithm::to_lower_copy(e1.name) < boost::algorithm::to_lower_copy(e2.name);
        else
        {
            output = boost::algorithm::to_lower_copy(e1.name) < boost::algorithm::to_lower_copy(e2.name);
        }
    }

    if (!e1.isDirectory && !e2.isDirectory)
    {
        switch (item)
        {
        case 0:
            output = boost::algorithm::to_lower_copy(e1.name) < boost::algorithm::to_lower_copy(e2.name);
            break;
        case 1:
            output = e1.last_modified < e2.last_modified;
            break;
        case 2:
            output = boost::filesystem::path(e1.name).extension().string() < boost::filesystem::path(e2.name).extension().string();
            break;
        case 3:
            output = e1.size < e2.size;
            break;

        default:
            return true;
        }
    }
    if (direction == 1)
        return output;
    else
        return !output;
}

void MergeSortedIntervals(std::vector<FilesystemEntry> &v, int s, int m, int e, int direction, int item)
{

    // temp is used to temporary store the vector obtained by merging
    // elements from [s to m] and [m+1 to e] in v
    std::vector<FilesystemEntry> temp;

    int i, j;
    i = s;
    j = m + 1;

    while (i <= m && j <= e)
    {
        // int compareResult = CompareEntries(v[i],v[j], direction, item);
        if (CompareEntries(v[i], v[j], direction, item))
        { //(v[i] <= v[j]) {
            temp.push_back(v[i]);
            ++i;
        }
        else
        {
            temp.push_back(v[j]);
            ++j;
        }
    }

    while (i <= m)
    {
        temp.push_back(v[i]);
        ++i;
    }

    while (j <= e)
    {
        temp.push_back(v[j]);
        ++j;
    }

    for (int i = s; i <= e; ++i)
        v[i] = temp[i - s];
}

// the MergeSort function
// Sorts the array in the range [s to e] in v using
// merge sort algorithm
void MergeSort(std::vector<FilesystemEntry> &v, int s, int e, int direction, int item)
{
    if (s < e)
    {
        int m = (s + e) / 2;
        MergeSort(v, s, m, direction, item);
        MergeSort(v, m + 1, e, direction, item);
        MergeSortedIntervals(v, s, m, e, direction, item);
    }
}

std::vector<FilesystemEntry> SortList(std::vector<FilesystemEntry> input, int direction, int item)
{
    MergeSort(input, 0, input.size() - 1, direction, item);
    return input;
}