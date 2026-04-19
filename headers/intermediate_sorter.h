// intermediate_sorter.h
// This header file declares the IntermediateSorter class, which
// groups mapper output into reducer-ready data. It provides the
// Aggregate(...) function that reads temporary mapper files and
// organizes identical words together. This file defines what
// IntermediateSorter can do, while intermediate_sorter.cpp contains
// the actual implementation.


#pragma once  // Include guard shortcut.

#include "file_manager.h"  // Used to verify that the temp directory exists.

#include <filesystem>  // Used for temp directory paths.
#include <map>         // Used to group a word with all of its values.
#include <string>      // Used for each word key.
#include <vector>      // Used to store each word's list of integer counts.

// IntermediateSorter handles the "shuffle/sort/group" stage in a simple local way.
// It reads the mapper temp files and groups all identical words together.
class IntermediateSorter {
public:
    // Constructor takes the shared FileManager by reference.
    explicit IntermediateSorter(FileManager& fileManager);

    // aggregate reads all temp chunk files and returns grouped data like:
    // "the" -> [1, 1, 1]
    // "cat" -> [1, 1]
    std::map<std::string, std::vector<int>> aggregate(const std::filesystem::path& temp_directory) const;

private:
    FileManager& file_manager_;  // Shared file helper from Workflow.
};
