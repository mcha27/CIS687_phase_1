// workflow.cpp
// Cross-platform MapReduce pipeline using dynamic Mapper plugin

#include "../headers/workflow.h"
#include <exception>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

// =========================================================
// Cross-platform dynamic loading macros
// =========================================================
#ifdef _WIN32
    #define LIB_HANDLE HMODULE
    #define LOAD_LIB(path) LoadLibrary(path)
    #define GET_SYM(lib, name) GetProcAddress(lib, name)
    #define CLOSE_LIB(lib) FreeLibrary(lib)
    #define LIB_EXT ".dll"
#else
    #define LIB_HANDLE void*
    #define LOAD_LIB(path) dlopen(path, RTLD_LAZY)
    #define GET_SYM(lib, name) dlsym(lib, name)
    #define CLOSE_LIB(lib) dlclose(lib)
    #define LIB_EXT ".so"
#endif

// =========================================================
// Function pointer types (same across platforms)
// =========================================================
using CreateMapperFunc = void* (*)(void*, const char*, std::size_t);
using DestroyMapperFunc = void (*)(void*);
using MapperMapFunc = bool (*)(void*, const char*, const char*);
using MapperFlushFunc = bool (*)(void*);

Workflow::Workflow() = default;

bool Workflow::run(
    const fs::path& input_directory,
    const fs::path& output_directory,
    const fs::path& temp_directory) {

    try {
        // Step 1: Validate input directory
        if (!file_manager_.directory_exists(input_directory)) {
            std::cerr << "Input directory does not exist: " << input_directory << std::endl;
            return false;
        }

        // Step 2: Ensure output directory
        if (!file_manager_.ensure_directory(output_directory)) {
            std::cerr << "Could not create/access output directory." << std::endl;
            return false;
        }

        // Step 3: Ensure temp directory
        if (!file_manager_.ensure_directory(temp_directory)) {
            std::cerr << "Could not create/access temp directory." << std::endl;
            return false;
        }

        // Step 4: Clear output directory
        if (!file_manager_.clear_directory_contents(output_directory)) {
            std::cerr << "Could not clear output directory." << std::endl;
            return false;
        }

        // Step 5: Clear temp directory
        if (!file_manager_.clear_directory_contents(temp_directory)) {
            std::cerr << "Could not clear temp directory." << std::endl;
            return false;
        }

        // Step 6: Reset output file
        if (!file_manager_.reset_final_output_file(output_directory, k_output_file_name)) {
            std::cerr << "Could not reset final output file." << std::endl;
            return false;
        }

        // Step 7: Get input files
        const auto input_files = file_manager_.get_input_files(input_directory);

        if (input_files.empty()) {
            std::cerr << "No input files found in: " << input_directory << std::endl;
            return false;
        }

        // =========================================================
        // STEP 8: LOAD MAPPER LIBRARY (cross-platform)
        // =========================================================
        std::string libPath = "bin/dll/mapper";
        libPath += LIB_EXT;

        LIB_HANDLE mapperLib = LOAD_LIB(libPath.c_str());

        if (!mapperLib) {
#ifdef _WIN32
            std::cerr << "Failed to load mapper DLL." << std::endl;
#else
            std::cerr << "Failed to load mapper SO: " << dlerror() << std::endl;
#endif
            return false;
        }

        auto CreateMapper = (CreateMapperFunc)GET_SYM(mapperLib, "CreateMapper");
        auto DestroyMapper = (DestroyMapperFunc)GET_SYM(mapperLib, "DestroyMapper");
        auto MapperMap = (MapperMapFunc)GET_SYM(mapperLib, "MapperMap");
        auto MapperFlush = (MapperFlushFunc)GET_SYM(mapperLib, "MapperFlush");

        if (!CreateMapper || !DestroyMapper || !MapperMap || !MapperFlush) {
            std::cerr << "Failed to load mapper symbols." << std::endl;
            CLOSE_LIB(mapperLib);
            return false;
        }

        // =========================================================
        // STEP 9: CREATE MAPPER INSTANCE
        // =========================================================
        void* mapper = CreateMapper(
            static_cast<void*>(&file_manager_),
            temp_directory.string().c_str(),
            1000
        );

        if (!mapper) {
            std::cerr << "Failed to create mapper instance." << std::endl;
            CLOSE_LIB(mapperLib);
            return false;
        }

        // =========================================================
        // STEP 10: RUN MAPPER
        // =========================================================
        for (const auto& file_path : input_files) {
            const auto lines = file_manager_.read_all_lines(file_path);

            for (const auto& line : lines) {
                if (!MapperMap(
                        mapper,
                        file_path.filename().string().c_str(),
                        line.c_str())) {

                    std::cerr << "Mapper failed while processing file: "
                              << file_path << std::endl;

                    DestroyMapper(mapper);
                    CLOSE_LIB(mapperLib);
                    return false;
                }
            }
        }

        // =========================================================
        // STEP 11: FLUSH MAPPER
        // =========================================================
        if (!MapperFlush(mapper)) {
            std::cerr << "Failed to flush mapper buffer." << std::endl;

            DestroyMapper(mapper);
            CLOSE_LIB(mapperLib);
            return false;
        }

        // =========================================================
        // STEP 12: SORT
        // =========================================================
        IntermediateSorter sorter(file_manager_);
        const auto grouped_data = sorter.aggregate(temp_directory);

        // =========================================================
        // STEP 13: REDUCE
        // =========================================================
        Reducer reducer(file_manager_, output_directory, k_output_file_name);

        for (const auto& entry : grouped_data) {
            if (!reducer.reduce(entry.first, entry.second)) {
                std::cerr << "Reducer failed for key: " << entry.first << std::endl;

                DestroyMapper(mapper);
                CLOSE_LIB(mapperLib);
                return false;
            }
        }

        // =========================================================
        // STEP 14: FINALIZE
        // =========================================================
        if (!reducer.finish()) {
            std::cerr << "Failed to finish reducer output." << std::endl;

            DestroyMapper(mapper);
            CLOSE_LIB(mapperLib);
            return false;
        }

        // =========================================================
        // CLEANUP
        // =========================================================
        DestroyMapper(mapper);
        CLOSE_LIB(mapperLib);

        std::cout << "MapReduce completed successfully." << std::endl;
        std::cout << "Final output file: "
                  << (output_directory / k_output_file_name) << std::endl;
        std::cout << "SUCCESS file created in: " << output_directory << std::endl;

        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Workflow error: " << ex.what() << std::endl;
        return false;
    }
}