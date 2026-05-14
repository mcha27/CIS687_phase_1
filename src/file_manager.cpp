#include "../headers/file_manager.h" 
#include <algorithm>
#include <fstream>    
#include <iostream>   
#include <sstream>  
#include <stdexcept>  

// Create a short alias for std::filesystem to keep the code easier to read.
namespace fs = std::filesystem;

// Check whether the path exists and is actually a directory.
bool FileManager::directory_exists(const fs::path& directory) const {
    return fs::exists(directory) && fs::is_directory(directory);
}

// Make sure a directory exists. If it does not, try to create it.
bool FileManager::ensure_directory(const fs::path& directory) const {
    try {
        if (!fs::exists(directory)) {
            fs::create_directories(directory);
        }
        return fs::exists(directory) && fs::is_directory(directory);
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to create/access directory: " << directory << "\nReason: " << ex.what() << std::endl;
        return false;
    }
}

// Delete everything inside a directory.
bool FileManager::clear_directory_contents(const fs::path& directory) const {
    try {
        // If the folder is not valid, we cannot clear it.
        if (!directory_exists(directory)) {
            return false;
        }

        // Loop through every entry in the directory.
        for (const auto& entry : fs::directory_iterator(directory)) {
            // remove_all deletes files or subfolders recursively.
            fs::remove_all(entry.path());
        }

        return true;  // Everything was cleared successfully.
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to clear directory: " << directory
                  << "\nReason: " << ex.what() << std::endl;
        return false;
    }
}

// Find regular files in the input directory and return them as a sorted list.
std::vector<fs::path> FileManager::get_input_files(const fs::path& input_directory) const {
    std::vector<fs::path> files;  // This will hold the files we discover.

    // If the input directory is invalid, just return an empty list.
    if (!directory_exists(input_directory)) {
        return files;
    }

    // Look through everything inside the input directory.
    for (const auto& entry : fs::directory_iterator(input_directory)) {
        // Only keep regular files, not folders.
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    // Sort the file list so runs are more predictable and easier to test.
    std::sort(files.begin(), files.end());
    return files;
}

// Read every line from one file and return those lines in a vector.
std::vector<std::string> FileManager::read_all_lines(const fs::path& file_path) const {
    std::vector<std::string> lines;   // Store each line we read.
    std::ifstream inputFile(file_path);  // Open the file for reading.

    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open input file: " + file_path.string());
    }

    std::string line;  // Temporary variable for one line at a time.

    // Read until end-of-file.
    while (std::getline(inputFile, line)) {
        lines.push_back(line);  // Save each line into the result vector.
    }

    return lines;
}

// Write one mapper chunk file such as chunk_0.txt into the temp directory.
bool FileManager::write_temp_chunk(
    const fs::path& temp_directory,
    int chunk_number,
    const std::vector<std::pair<std::string, int>>& records) const {

    try {
        // Build a chunk file path like temp/chunk_0.txt.
        fs::path chunkPath = temp_directory / ("chunk_" + std::to_string(chunk_number) + ".txt");

        // Open the chunk file for writing.
        std::ofstream outputFile(chunkPath);

        // If it does not open, return false.
        if (!outputFile.is_open()) {
            std::cerr << "Could not open temp chunk file: " << chunkPath << std::endl;
            return false;
        }

        // Write each mapper record as: word TAB 1
        for (const auto& record : records) {
            outputFile << record.first << '\t' << record.second << '\n';
        }

        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to write temp chunk. Reason: " << ex.what() << std::endl;
        return false;
    }
}

// Append final reduced results to the final output file.
bool FileManager::append_final_results(
    const fs::path& output_directory,
    const std::string& output_file_name,
    const std::vector<std::pair<std::string, int>>& records) const {

    try {
        // Build the full path to the final output file.
        fs::path outputPath = output_directory / output_file_name;

        // Open in append mode so new results are added to the end.
        std::ofstream outputFile(outputPath, std::ios::app);

        if (!outputFile.is_open()) {
            std::cerr << "Could not open final output file: " << outputPath << std::endl;
            return false;
        }

        // Write each final reduced pair as: word TAB totalCount
        for (const auto& record : records) {
            outputFile << record.first << '\t' << record.second << '\n';
        }

        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to append final output. Reason: " << ex.what() << std::endl;
        return false;
    }
}

// Reset the final output file so it starts empty.
bool FileManager::reset_final_output_file(
    const fs::path& output_directory,
    const std::string& output_file_name) const {

    try {
        // Build the path to the final output file.
        fs::path outputPath = output_directory / output_file_name;

        // Open in truncate mode so any old content is erased.
        std::ofstream outputFile(outputPath, std::ios::trunc);

        // Return true if the file opened correctly.
        return outputFile.is_open();
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to reset output file. Reason: " << ex.what() << std::endl;
        return false;
    }
}

// Create the required empty SUCCESS file in the output directory.
bool FileManager::create_success_file(const fs::path& output_directory) const {
    try {
        // Build the full SUCCESS file path.
        fs::path successPath = output_directory / "SUCCESS";

        // Open the file in truncate mode.
        // That creates it if missing and clears it if it already exists.
        std::ofstream successFile(successPath, std::ios::trunc);

        // Return true if the file opened successfully.
        return successFile.is_open();
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to create SUCCESS file. Reason: " << ex.what() << std::endl;
        return false;
    }
}
