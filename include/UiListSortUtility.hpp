#include "fss-class.hpp"
#include "file-list.hpp"

// the interval from [s to m] and [m+1 to e] in v are sorted
// the function will merge both of these intervals
// such the interval from [s to e] in v becomes sorted

// true = in right order, false = in wrong order
bool CompareEntries(FilesystemEntry e1, FilesystemEntry e2, int direction, int item);

void MergeSortedIntervals(std::vector<FilesystemEntry> &v, int s, int m, int e, int direction, int item);

// the MergeSort function
// Sorts the array in the range [s to e] in v using
// merge sort algorithm
void MergeSort(std::vector<FilesystemEntry> &v, int s, int e, int direction, int item);

std::vector<FilesystemEntry> SortList(std::vector<FilesystemEntry> input, int direction, int item);