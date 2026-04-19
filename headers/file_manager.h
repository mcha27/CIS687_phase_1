#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector> 

// FileManager is the only class that should directly deal with the file system. That matches the project requirement that file system access be abstracted away.
class FileManager {
public:
    // Checks whether a directory exists and is actually a directory.
    bool directory_exists(const std::filesystem::path& directory) const;

    // Creates the directory if it does not exist yet.
    // Returns true if the directory exists at the end of the call.
    bool ensure_directory(const std::filesystem::path& directory) const;

    // Deletes everything inside a directory, but keeps the directory itself.
    // We use this so each program run starts with clean temp/output folders.
    bool clear_directory_contents(const std::filesystem::path& directory) const;

    // Returns a list of regular files found in the input directory.
    std::vector<std::filesystem::path> get_input_files(const std::filesystem::path& input_directory) const;

    // Reads every line from one text file and returns those lines in a vector.
    std::vector<std::string> read_all_lines(const std::filesystem::path& file_path) const;

    // Writes one mapper chunk file into the temp directory. records contains pairs like ("word", 1).
    bool write_temp_chunk(const std::filesystem::path& temp_directory, int chunk_number, const std::vector<std::pair<std::string, int>>& records) const;

    // Appends final reducer results to the final output file. records contains pairs like ("word", totalCount).
    bool append_final_results(const std::filesystem::path& output_directory, const std::string& output_file_name, const std::vector<std::pair<std::string, int>>& records) const;

    // Clears or creates the final output file before the reducer starts writing into it.
    bool reset_final_output_file( const std::filesystem::path& output_directory, const std::string& output_file_name) const;

    // Creates the empty SUCCESS file required 
    bool create_success_file(const std::filesystem::path& output_directory) const;
};
