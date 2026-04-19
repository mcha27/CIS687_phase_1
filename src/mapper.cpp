/*
Map Class: The map class will contain a public method map(), that accepts a key and value. 
The key will be the file name and the value will be a single line of raw data from the file. 
The map() function will tokenize the value into distinct words (remove punctuation, whitespace and capitalization). 
The map() function will call a second function export() that takes a key and value as two parameters. 
The export function will buffer output in memory and periodically write the data out to disk (periodicity can be based on the size of the buffer). 
The intermediate data will be written to the temporary directory (specified via command line). 
*/

#include "mapper.h" 
#include <cctype> 
#include <sstream>

// Mapper constructor:
// Sets up references and settings needed for the map phase.
Mapper::Mapper(FileManager& fileManager, const std::filesystem::path& temp_directory, std::size_t max_buffer_size)
    : file_manager_(fileManager),       // Save the shared FileManager reference.
      temp_directory_(temp_directory),   // Save the temp directory path for future chunk writes.
      max_buffer_size_(max_buffer_size),   // Save how large the in-memory buffer may grow before flushing.
      chunk_counter_(0) {               // Start the chunk counter at 0 for chunk_0.txt.
}

// map handles one raw line from one file.
bool Mapper::map(const std::string& file_name, const std::string& raw_line) {
    (void)file_name;
    // Convert the raw line into normalized tokens: "Hello, WORLD!" -> ["hello", "world"]
    std::vector<std::string> tokens = tokenize_and_normalize(raw_line);

    // Export each token as a ("word", 1) record.
    for (const auto& token : tokens) {
        if (!export_record(token, 1)) {
            return false; 
        }
    }

    return true; 
}

// Turn one raw line into clean lowercase tokens with punctuation removed.
std::vector<std::string> Mapper::tokenize_and_normalize(const std::string& raw_line) const {
    std::string cleaned_line;          
    cleaned_line.reserve(raw_line.size());

    // Look at each character in the original line.
    for (unsigned char ch : raw_line) {
        // If the character is a letter or number, keep it and lowercase it.
        if (std::isalnum(ch)) {
            cleaned_line.push_back(static_cast<char>(std::tolower(ch)));
        }
        else {
 
            cleaned_line.push_back(' ');
        }
    }

    // Feed the cleaned line into a string stream so we can extract words using >>.
    std::stringstream stream(cleaned_line);
    std::vector<std::string> tokens;  // Final list of words from this line.
    std::string token;                // Temporary variable for one token at a time.

    // Extract words separated by spaces.
    while (stream >> token) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens; 
}

// Save one mapper record into the in-memory buffer.
bool Mapper::export_record(const std::string& key, int value) {
    // Store the pair (word, 1) in the buffer.
    buffer_.push_back({ key, value });

    // If the buffer reached the allowed max size, flush it to disk now.
    if (buffer_.size() >= max_buffer_size_) {
        return flush_buffer();
    }

    return true;  // Buffer accepted the new record.
}

// Write the current mapper buffer to a temp chunk file.
bool Mapper::flush_buffer() {
    if (buffer_.empty()) {
        return true;
    }
    // Ask FileManager to write the current buffer into chunk_N.txt.
    const bool write_ok = file_manager_.write_temp_chunk(temp_directory_, chunk_counter_, buffer_);
    if (!write_ok) {
        return false;  // Stop if the write failed.
    }

    ++chunk_counter_;   // Move to the next chunk number for the next flush.
    buffer_.clear();   // Clear the in-memory records because they are now on disk.
    return true;
}
