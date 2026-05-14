// executive.h
// This header file declares the Executive class, which handles
// program startup and command-line processing. It provides the
// run(...) function that validates and interprets directory inputs
// and starts the workflow. This file defines what Executive can do,
// while executive.cpp contains the actual implementation.

#pragma once

#include <string>

// The Executive class is responsible for:
// 1) Parsing command-line arguments
// 2) Applying default values for optional directories
// 3) Starting the Workflow execution
class Executive {
public:
    // Entry point for the program after main().
    //
    // Supported CLI formats:
    //   MapReduce.exe <input>
    //   MapReduce.exe <input> <output>
    //   MapReduce.exe <input> <output> <temp>
    //
    // Returns:
    //   0 = success
    //   1 = failure
    int run(int argc, char* argv[]);
};