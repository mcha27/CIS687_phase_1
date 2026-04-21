// executive.h
// This header file declares the Executive class, which handles
// program startup and command-line processing. It provides the
// run(...) function that validates the required directory inputs
// and starts the workflow. This file defines what Executive can do,
// while executive.cpp contains the actual implementation.

#pragma once  // Tells the compiler to include this header file only one time.

// main.cpp creates an Executive object and tells it to run.
// Executive does two jobs:
// 1) read the command-line arguments from the user
// 2) pass those folder paths to Workflow 
class Executive {
public:
    // run is the public entry point for this class.
    // argc = how many command-line arguments were passed in
    // argv = the actual argument values as character arrays
    // It returns 0 when the program succeeds and 1 when something fails.
    int run(int argc, char* argv[]);
};
