#pragma once

#include "file_manager.h" 

#include <filesystem> 
#include <string> 
#include <utility>  
#include <vector>  

// Mapper handles the "map" phase of MapReduce. It receives one line at a time, breaks that line into clean words, and turns each word into a key/value pair like ("hello", 1).
class Mapper {
    public:
        // Constructor:
        // - fileManager lets Mapper write temp chunk files without doing raw file I/O itself
        // - temp_directory tells Mapper where intermediate chunk files should go
        // - max_buffer_size controls when the in-memory buffer should flush to disk
        Mapper(FileManager& fileManager, const std::filesystem::path& temp_directory, std::size_t max_buffer_size = 1000);

        // file_name is included because the assignment says map() accepts a key and value, where the key is the source file name and the value is one line of text.
        bool map(const std::string& file_name, const std::string& raw_line);

        // flush_buffer forces any remaining buffered mapper output to be written to disk.
        bool flush_buffer();

    private:
        // Helper that converts one raw text line into a list of normalized words.
        std::vector<std::string> tokenize_and_normalize(const std::string& raw_line) const;

        // Helper that stores one ("word", 1) record in memory and flushes if needed.
        bool export_record(const std::string& key, int value);

        FileManager& file_manager_;  // Reference to the shared FileManager used by the whole workflow.
        std::filesystem::path temp_directory_;  // Where chunk files should be written.
        std::vector<std::pair<std::string, int>> buffer_;  // In-memory mapper output waiting to be flushed.
        std::size_t max_buffer_size_;  // How many records can sit in memory before we flush to disk.
        int chunk_counter_;  // Used to create unique chunk file names such as chunk_0.txt, chunk_1.txt, etc.
    };
