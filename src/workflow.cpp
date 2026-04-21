// workflow.cpp
// This file is responsible for coordinating the full processing pipeline.
// It validates the input_directory, output_directory, and temp_directory,
// then controls the program flow by calling the file_manager, mapper,
// intermediate_sorter, and reducer components in the correct order.
// It does not perform every task itself; instead, it manages the overall
// word-count workflow from input processing through final output creation.


#include "workflow.h"  
#include <exception>    
#include <iostream>     

// This creates a shorter alias so we can write fs::path instead of std::filesystem::path everywhere.
namespace fs = std::filesystem;

// The default constructor does not need special setup right now.
Workflow::Workflow() = default;

// run coordinates the entire program from start to finish.
bool Workflow::run(
    const fs::path& input_directory,
    const fs::path& output_directory,
    const fs::path& temp_directory) {

    try {
        // Step 1: Make sure the input folder exists.
        // If it does not exist, there is nothing to read.
        if (!file_manager_.directory_exists(input_directory)) {
            std::cerr << "Input directory does not exist: " << input_directory << std::endl;
            return false;
        }

        // Step 2: Make sure the output folder exists or can be created.
        // The reducer will need this later to write the final answer.
        if (!file_manager_.ensure_directory(output_directory)) {
            std::cerr << "Could not create/access output directory." << std::endl;
            return false;
        }

        // Step 3: Make sure the temp folder exists or can be created.
        // The mapper will store chunk files here.
        if (!file_manager_.ensure_directory(temp_directory)) {
            std::cerr << "Could not create/access temp directory." << std::endl;
            return false;
        }

        // Step 4: Clear the old output folder contents so each run starts fresh.
        // This prevents old files from mixing with the current run.
        if (!file_manager_.clear_directory_contents(output_directory)) {
            std::cerr << "Could not clear output directory." << std::endl;
            return false;
        }

        // Step 5: Clear the old temp folder contents for the same reason.
        if (!file_manager_.clear_directory_contents(temp_directory)) {
            std::cerr << "Could not clear temp directory." << std::endl;
            return false;
        }

        // Step 6: Reset the final output file so it starts empty before reducer output is appended.
        if (!file_manager_.reset_final_output_file(output_directory, k_output_file_name)) {
            std::cerr << "Could not reset final output file." << std::endl;
            return false;
        }

        // Step 7: Get all regular files from the input directory.
        const auto input_files = file_manager_.get_input_files(input_directory);

        // If there are no files to process, we stop here with a clear message.
        if (input_files.empty()) {
            std::cerr << "No input files found in: " << input_directory << std::endl;
            return false;
        }

        // Step 8: Create the Mapper.
        // We pass FileManager so the mapper can flush chunk files using that class.
        Mapper mapper(file_manager_, temp_directory);

        // Step 9: Loop through every input file.
        for (const auto& file_path : input_files) {
            // Read all lines from the current file.
            const auto lines = file_manager_.read_all_lines(file_path);

            // Loop through each line and hand it to the mapper.
            for (const auto& line : lines) {
                // file_path.filename().string() sends just the file name, not the full path.
                if (!mapper.map(file_path.filename().string(), line)) {
                    std::cerr << "Mapper failed while processing file: " << file_path << std::endl;
                    return false;
                }
            }
        }

        // Step 10: Force the mapper to flush anything still left in memory.
        // Without this, the last partial chunk could be lost.
        if (!mapper.flush_buffer()) {
            std::cerr << "Failed to flush mapper buffer." << std::endl;
            return false;
        }

        // Step 11: Create the sorter/aggregator and group mapper output by word.
        IntermediateSorter sorter(file_manager_);
        const auto grouped_data = sorter.aggregate(temp_directory);

        // Step 12: Create the reducer that will write final output.
        Reducer reducer(file_manager_, output_directory, k_output_file_name);

        // Step 13: Reduce one grouped entry at a time.
        for (const auto& entry : grouped_data) {
            // entry.first is the word
            // entry.second is the vector of counts for that word
            if (!reducer.reduce(entry.first, entry.second)) {
                std::cerr << "Reducer failed for key: " << entry.first << std::endl;
                return false;
            }
        }

        // Step 14: Flush remaining reducer output and create the SUCCESS file.
        if (!reducer.finish()) {
            std::cerr << "Failed to finish reducer output." << std::endl;
            return false;
        }

        // Print success messages so the user knows where the output went.
        std::cout << "MapReduce completed successfully." << std::endl;
        std::cout << "Final output file: " << (output_directory / k_output_file_name) << std::endl;
        std::cout << "SUCCESS file created in: " << output_directory << std::endl;
        return true;  // Signal success back to Executive.
    }
    catch (const std::exception& ex) {
        // Catch any exception that escaped lower-level code.
        // This helps us fail safely instead of crashing.
        std::cerr << "Workflow error: " << ex.what() << std::endl;
        return false;
    }
}
