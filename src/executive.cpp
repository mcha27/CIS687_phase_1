// executive.cpp
// This file is responsible for startup and command-line handling.
// It validates the required input_directory, output_directory,
// and temp_directory arguments, then creates the Workflow object
// and starts the MapReduce process by calling workflow.run(...).
// It does not perform the word counting itself; it simply initializes
// the program and transfers control to workflow.cpp.


#include "../headers/executive.h"
#include "../headers/workflow.h"   
#include <filesystem>   
#include <iostream>    

// run is the first real program logic after main().
int Executive::run(int argc, char* argv[]) {
    // The assignment requires three directory arguments:
    // 1) input directory
    // 2) output directory
    // 3) temp directory
    // argc includes the program name itself, so we expect 4 total items.
    if (argc != 4) {
        // Print a clear usage message so the user knows exactly how to run the program.
        std::cerr << "Usage: MapReduce.exe <input_directory> <output_directory> <temp_directory>" << std::endl;
        return 1;  // Return a non-zero code because the program cannot continue.
    }

    // Convert the three command-line strings into filesystem path objects.
    // This makes path handling cleaner and safer in the rest of the program.
    const std::filesystem::path input_directory = argv[1];
    const std::filesystem::path output_directory = argv[2];
    const std::filesystem::path temp_directory = argv[3];

    // Create the Workflow object.
    // Workflow is the part that actually runs the MapReduce pipeline.
    Workflow workflow;

    // Start the full processing pipeline using the three paths.
    const bool success = workflow.run(input_directory, output_directory, temp_directory);

    // Return 0 if everything worked, otherwise return 1.
    return success ? 0 : 1;
}
