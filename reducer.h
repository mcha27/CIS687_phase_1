#pragma once  // Prevents duplicate inclusions of header files during compilation.

#include "file_manager.h"  // Need FileManager to write final output and SUCCESS.

#include <filesystem>  // Used for working with file directories.
#include <string>
#include <utility>     // Used for std::pair.
#include <vector>      // Dynamically sized arrays.

// Reducer receives a key and value (aka word and a list of its counts e.g. "a", [1,1,1]), then sums the counts together (e.g. "a", 3).
class Reducer {
public:
    // Constructor, simpler way to define class parameters
    Reducer(
        FileManager& fileManager,                       // Need FileManager to write final output and SUCCESS.
        const std::filesystem::path& output_directory,  // Where final .txt output is written to.
        const std::string& output_file_name,            // Name of final .txt output.
        std::size_t max_buffer_size = 500);             // Buffer size.

    // Reduce method takes a key and its values, sums values, then passes key and sum to export_record().
    // e.g. key = "a"; values = [1, 1, 1]; sum = 3
    bool reduce(const std::string& key, const std::vector<int>& values);

    // Assuming no more appends to output file after this:
    // Append contents of buffer into output file, flush buffer, then create SUCCESS file.
    bool finish();

private:
    // Add key and sum to buffer. Check if buffer is full.
    bool export_record(const std::string& key, int reduced_value);

    // Append contents of buffer into output file, then empty buffer.
    bool flush_buffer();

    FileManager& file_manager_;                         // Shared FileManager from Workflow.
    std::filesystem::path output_directory_;            // File directory where final .txt output file will be saved.
    std::string output_file_name_;                      // Name of output file.
    std::vector<std::pair<std::string, int>> buffer_;   // Buffer.
    std::size_t max_buffer_size_;                       // Buffer size.
};
