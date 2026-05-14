// intermediate_sorter.cpp
// This file is responsible for the sorting and aggregation stage
// of the program. It reads the intermediate mapper chunk files
// from the temporary directory and groups matching words together
// into collections of values. The grouped output produced here
// is then passed to the reducer for final counting.


#include "../headers/intermediate_sorter.h"  // Include the matching class declaration.
#include <filesystem>  // Needed for directory iteration in the temp folder.
#include <fstream>     // Needed for std::ifstream to read temp chunk files.
#include <iostream>    // Needed for warning messages if a temp file cannot be read.
#include <sstream>     // Needed for parsing lines like "word<TAB>1".

// Create a short alias for std::filesystem to keep code cleaner.
namespace fs = std::filesystem;

// Constructor stores the shared FileManager reference.
IntermediateSorter::IntermediateSorter(FileManager& fileManager)
    : file_manager_(fileManager) {
}

// aggregate reads mapper chunk files and groups counts by word.
std::map<std::string, std::vector<int>> IntermediateSorter::aggregate(const fs::path& temp_directory) const {
    // grouped_data will end up looking like:
    // "cat" -> [1, 1]
    // "the" -> [1, 1, 1]
    std::map<std::string, std::vector<int>> grouped_data;

    // If the temp directory does not exist, return an empty grouped result.
    if (!file_manager_.directory_exists(temp_directory)) {
        return grouped_data;
    }

    // Loop through every file in the temp directory.
    for (const auto& entry : fs::directory_iterator(temp_directory)) {
        // Skip anything that is not a regular file.
        if (!entry.is_regular_file()) {
            continue;
        }

        // Open the temp chunk file.
        std::ifstream inputFile(entry.path());

        // If this file cannot be opened, warn the user and continue with the others.
        if (!inputFile.is_open()) {
            std::cerr << "Skipping unreadable temp file: " << entry.path() << std::endl;
            continue;
        }

        std::string line;  // Holds one temp-file line at a time.

        // Read every line from this chunk file.
        while (std::getline(inputFile, line)) {
            // Each line should look like: word<TAB>1
            std::stringstream stream(line);
            std::string word;
            int value = 0;

            // Read the word up to the tab, then read the integer after it.
            if (std::getline(stream, word, '\t') && (stream >> value)) {
                // Add the value to this word's list in the map.
                grouped_data[word].push_back(value);
            }
        }
    }

    // Return the grouped structure to Workflow so Reducer can use it.
    return grouped_data;
}
