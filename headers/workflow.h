// workflow.h
// This header file declares the Workflow class, which manages the
// overall processing pipeline for the program. It provides the
// run(...) function that coordinates input validation, mapping,
// sorting, reducing, and final output creation. This file defines
// what Workflow can do, while workflow.cpp contains the actual
// implementation.

#pragma once  // Prevent this header from being included more than once.

// These includes let Workflow talk to the other major components.
// Workflow owns or creates these objects and calls their methods in order.
#include "file_manager.h"
#include "intermediate_sorter.h"
#include "mapper.h"
#include "reducer.h"

#include <filesystem>  // Gives us std::filesystem::path for folders and files.
#include <string>      // Gives us std::string for file names and text.


class Workflow {
public:
    // Default constructor.
    // We do not need to pass anything in when creating Workflow.
    Workflow();

    // run executes the entire MapReduce pipeline:
    // 1) verify folders
    // 2) clean temp/output folders
    // 3) read input files
    // 4) call Mapper on each line
    // 5) aggregate temp results
    // 6) call Reducer
    // 7) create SUCCESS
    bool run(
        const std::filesystem::path& input_directory,   // Folder containing the original text files
        const std::filesystem::path& output_directory,  // Folder where final results will be written
        const std::filesystem::path& temp_directory);   // Folder where mapper chunk files will be written

private:
    // FileManager is stored as a member variable because all file I/O should go through it.
    FileManager file_manager_;

    // This constant stores the final output file name in one place so it is easy to reuse.
    static constexpr const char* k_output_file_name = "word_counts.txt";
};
