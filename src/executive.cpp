#include "../headers/executive.h"
#include "../headers/workflow.h"

#include <filesystem>
#include <iostream>

int Executive::run(int argc, char* argv[]) {

    // Default directories
    const std::filesystem::path default_output_directory = "output";
    const std::filesystem::path default_temp_directory = "temp";

    // Minimum requirement: input directory only
    if (argc < 2 || argc > 4) {
        std::cerr
            << "Usage:\n"
            << "  MapReduce.exe <input_directory>\n"
            << "  MapReduce.exe <input_directory> <output_directory>\n"
            << "  MapReduce.exe <input_directory> <output_directory> <temp_directory>\n";

        return 1;
    }

    const std::filesystem::path input_directory = argv[1];

    // Resolve output directory
    std::filesystem::path output_directory =
        (argc >= 3)
            ? argv[2]
            : default_output_directory;

    // Resolve temp directory
    std::filesystem::path temp_directory =
        (argc == 4)
            ? argv[3]
            : default_temp_directory;

    Workflow workflow;

    const bool success =
        workflow.run(
            input_directory,
            output_directory,
            temp_directory
        );

    return success ? 0 : 1;
}