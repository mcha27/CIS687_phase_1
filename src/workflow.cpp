// workflow.cpp
// Cross-platform MapReduce pipeline using dynamic Mapper + Reducer plugins

#include "../headers/workflow.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

// Cross-platform dynamic loading macros
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


// Mapper function pointer types
using CreateMapperFunc = void* (*)(void*, const char*, std::size_t);
using DestroyMapperFunc = void (*)(void*);
using MapperMapFunc = bool (*)(void*, const char*, const char*);
using MapperFlushFunc = bool (*)(void*);

// Reducer function pointer types
using CreateReducerFunc = void* (*)(void*, const char*, const char*, std::size_t);
using DestroyReducerFunc = void (*)(void*);
using ReducerReduceFunc = bool (*)(void*, const char*, const int*, std::size_t);
using ReducerFinishFunc = bool (*)(void*);

Workflow::Workflow() = default;

bool Workflow::run(const fs::path& input_directory, const fs::path& output_directory, const fs::path& temp_directory) {
    try {
        // VALIDATE INPUT DIRECTORY
        if (!file_manager_.directory_exists(input_directory)) {
            std::cerr << "Input directory does not exist: " << input_directory << std::endl;
            return false;
        }

        // ENSURE OUTPUT DIRECTORY EXISTS
        if (!file_manager_.ensure_directory(output_directory)) {
            std::cerr << "Could not create/access output directory." << std::endl;
            return false;
        }

        // ENSURE TEMP DIRECTORY EXISTS
        if (!file_manager_.ensure_directory(temp_directory)) {
            std::cerr << "Could not create/access temp directory." << std::endl;
            return false;
        }

        // CLEAR OUTPUT DIRECTORY
        if (!file_manager_.clear_directory_contents(output_directory)) {
            std::cerr << "Could not clear output directory." << std::endl;
            return false;
        }

        // CLEAR TEMP DIRECTORY
        if (!file_manager_.clear_directory_contents(temp_directory)) {
            std::cerr << "Could not clear temp directory." << std::endl;
            return false;
        }

        // RESET OUPUT FILES
        if (!file_manager_.reset_final_output_file(output_directory, k_output_file_name)) {
            std::cerr << "Could not reset final output file." << std::endl;
            return false;
        }

        // GET INPUT FILES
        const auto input_files = file_manager_.get_input_files(input_directory);

        if (input_files.empty()) {
            std::cerr << "No input files found in: " << input_directory << std::endl;
            return false;
        }

        // LOAD MAPPER LIBRARY
        std::string mapperLibPath = "bin/dll/mapper";
        mapperLibPath += LIB_EXT;

        LIB_HANDLE mapperLib = LOAD_LIB(mapperLibPath.c_str());

        if (!mapperLib) {

#ifdef _WIN32
            std::cerr << "Failed to load mapper library."
                      << std::endl;
#else
            std::cerr << "Failed to load mapper library: "
                      << dlerror() << std::endl;
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

        // CREATE MAPPER INSTANCE
        void* mapper = CreateMapper(
            static_cast<void*>(&file_manager_),
            temp_directory.string().c_str(),
            1000
        );

        if (!mapper) {
            std::cerr << "Failed to create mapper instance."
                      << std::endl;

            CLOSE_LIB(mapperLib);
            return false;
        }

        // RUN MAPPER
        for (const auto& file_path : input_files) {
            const auto lines = file_manager_.read_all_lines(file_path);
            for (const auto& line : lines) {
                if (!MapperMap(mapper, file_path.filename().string().c_str(), line.c_str())) {
                    std::cerr << "Mapper failed while processing file: " << file_path << std::endl;
                    DestroyMapper(mapper);
                    CLOSE_LIB(mapperLib);
                    return false;
                }
            }
        }

        // FLUSH MAPPER
        if (!MapperFlush(mapper)) {
            std::cerr << "Failed to flush mapper buffer." << std::endl;
            DestroyMapper(mapper);
            CLOSE_LIB(mapperLib);
            return false;
        }

        // SORT / GROUP
        IntermediateSorter sorter(file_manager_);

        const auto grouped_data = sorter.aggregate(temp_directory);

        // LOAD REDUCER LIBRARY
        std::string reducerLibPath = "bin/dll/reducer";
        reducerLibPath += LIB_EXT;

        LIB_HANDLE reducerLib = LOAD_LIB(reducerLibPath.c_str());

        if (!reducerLib) {

#ifdef _WIN32
            std::cerr << "Failed to load reducer library."
                      << std::endl;
#else
            std::cerr << "Failed to load reducer library: "
                      << dlerror() << std::endl;
#endif

            DestroyMapper(mapper);
            CLOSE_LIB(mapperLib);
            return false;
        }

        auto CreateReducer = (CreateReducerFunc)GET_SYM(reducerLib, "CreateReducer");
        auto DestroyReducer = (DestroyReducerFunc)GET_SYM(reducerLib, "DestroyReducer");
        auto ReducerReduce = (ReducerReduceFunc)GET_SYM(reducerLib, "ReducerReduce");
        auto ReducerFinish = (ReducerFinishFunc)GET_SYM(reducerLib, "ReducerFinish");

        if (!CreateReducer || !DestroyReducer || !ReducerReduce || !ReducerFinish) {
            std::cerr << "Failed to load reducer symbols." << std::endl;
            DestroyMapper(mapper);
            CLOSE_LIB(mapperLib);
            CLOSE_LIB(reducerLib);
            return false;
        }

        //CREATE REDUCER INSTANCE
        void* reducer = CreateReducer(
            static_cast<void*>(&file_manager_),
            output_directory.string().c_str(),
            k_output_file_name,
            500
        );

        if (!reducer) {

            std::cerr << "Failed to create reducer instance."
                      << std::endl;

            DestroyMapper(mapper);

            CLOSE_LIB(mapperLib);
            CLOSE_LIB(reducerLib);

            return false;
        }

        //RUN REDUCER
        for (const auto& entry : grouped_data) {

            const std::string& key = entry.first;
            const std::vector<int>& values = entry.second;

            if (!ReducerReduce(
                    reducer,
                    key.c_str(),
                    values.data(),
                    values.size())) {

                std::cerr << "Reducer failed for key: "
                          << key << std::endl;

                DestroyMapper(mapper);
                DestroyReducer(reducer);

                CLOSE_LIB(mapperLib);
                CLOSE_LIB(reducerLib);

                return false;
            }
        }

        // FINISH REDUCER
        if (!ReducerFinish(reducer)) {

            std::cerr << "Failed to finish reducer output."
                      << std::endl;

            DestroyMapper(mapper);
            DestroyReducer(reducer);

            CLOSE_LIB(mapperLib);
            CLOSE_LIB(reducerLib);

            return false;
        }
        //CLEANUP
        DestroyMapper(mapper);
        DestroyReducer(reducer);

        CLOSE_LIB(mapperLib);
        CLOSE_LIB(reducerLib);

        // SUCCESS
        std::cout << "MapReduce completed successfully." << std::endl;
        std::cout << "Final output file: " << (output_directory / k_output_file_name) << std::endl;
        std::cout << "SUCCESS file created in: " << output_directory<< std::endl;
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Workflow error: " << ex.what() << std::endl;
        return false;
    }
}