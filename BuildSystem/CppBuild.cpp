#pragma once

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

template<typename ClockType = std::chrono::high_resolution_clock>
class Timer
{
public:
    static std::string CurrentTimeString()
    {
        auto now = ClockType::to_time_t(ClockType::now());
        std::stringstream time_string_stream;
        time_string_stream << std::ctime(&now);
        std::string time_string = time_string_stream.str();
        // Remove annoying final newline characters.
        time_string.pop_back();
        return time_string;
    }

    explicit Timer(const std::string& name) :
        Name(name),
        StartTime(ClockType::now())
    {}

    ~Timer()
    {
        //std::cout << GetElapsedTimeText() << std::endl;
    }

    std::string GetElapsedTimeText() const
    {
        auto current_time = ClockType::now();
        auto elapsed_time = current_time - StartTime;
        std::string elapsed_time_text = (
            /// @todo Name + ": " +
            std::to_string(elapsed_time.count()) + "\t" +
            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count()) + "ms\t" +
            std::to_string(std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count()) + "s");
        return elapsed_time_text;
    }

    std::string Name = {};
    std::chrono::time_point<ClockType> StartTime = {};
};

using HighResolutionTimer = Timer<std::chrono::high_resolution_clock>;
using SystemClockTimer = Timer<std::chrono::system_clock>;

/// A command that can be executed on the command line.
class Command
{
public:
    /// Executes the command.
    /// @return The return code from executing the command.
    int Execute()
    {
        // FORM THE STRING VERSION OF THE COMMAND.
        std::stringstream command;

        std::size_t component_count = Components.size();
        for (std::size_t component_index = 0; component_index < component_count; ++component_index)
        {
            // APPEND THE CURRENT COMPONENT OF THE COMMAND.
            // If the current component contains spaces, it must be quoted.
            std::string current_component = Components[component_index];
            std::size_t space_index_in_current_component = current_component.find(' ');
            bool current_component_contains_spaces = (std::string::npos != space_index_in_current_component);
            if (current_component_contains_spaces)
            {
                current_component = "\"" + current_component + "\"";
            }
            command << current_component;

            // APPEND A SPACE BEFORE ANY NEXT COMMAND COMPONENT.
            // Individual components need to be separated by spaces.
            // This is needed to ensure different components are properly separated by spaces.
            std::size_t max_component_index = component_count - 1;
            bool more_components_exist = (component_index < max_component_index);
            if (more_components_exist)
            {
                command << " ";
            }
        }

        // EXECUTE THE COMMAND.
        std::string command_string = command.str();
        SystemClockTimer command_timer(command_string);
        std::cout << SystemClockTimer::CurrentTimeString() << " - Executing: " << command_string << std::endl;
        int return_code = std::system(command_string.c_str());
        std::cout 
            << SystemClockTimer::CurrentTimeString() 
            << " - Execution of " << command_string 
            << " finished with return code " << return_code
            << " after " << command_timer.GetElapsedTimeText() << std::endl;
        return return_code;
    }

    /// The different components of the command, to be separated by spaces.
    std::vector<std::string> Components = {};
};

/// The different types of projects.
enum class ProjectType
{
    /// Identifies an invalid type of project.
    INVALID = 0,
    /// A library (be it static or dynamic).
    LIBRARY,
    /// An executable program.
    PROGRAM
};

/// A project that can be built.
class Project
{
public:
    void AddIncludeFolderPaths(std::vector<std::string>& command_line_arguments) const
    {
        for (const Project* library : Libraries)
        {
            // ADD DIRECTORIES FROM THE IMMEDIATE LIBRARY.
            command_line_arguments.push_back("/I");
            command_line_arguments.push_back(library->CodeFolderPath.string());
            command_line_arguments.push_back("/I");
            command_line_arguments.push_back(library->CodeFolderPath.parent_path().string());

            // ADD DIRECTORIES FROM ADDITIONAL DEPENDENCIES.
            library->AddIncludeFolderPaths(command_line_arguments);
        }
    }

    void AddLinkerFolderPaths(std::vector<std::string>& command_line_arguments) const
    {
        for (const Project* library : Libraries)
        {
            // ADD DIRECTORIES FROM THE IMMEDIATE LIBRARY.
            if (!library->CodeFolderPath.empty())
            {
                command_line_arguments.push_back("/LIBPATH:" + library->CodeFolderPath.string());
            }

            // ADD DIRECTORIES FROM ADDITIONAL DEPENDENCIES.
            library->AddLinkerFolderPaths(command_line_arguments);
        }
    }

    std::vector<std::string> GetLinkerLibraryInputs() const
    {
        std::vector<std::string> linker_library_names = LinkerLibraryNames;

        for (const Project* library : Libraries)
        {
            std::vector<std::string> dependency_libraries = library->GetLinkerLibraryInputs();
            for (const std::string& linker_input : dependency_libraries)
            {
                linker_library_names.push_back(linker_input);
            }
        }

        return linker_library_names;
    }

    /// Builds the project.
    void Build(const std::filesystem::path& build_folder_path)
    {
        SystemClockTimer command_timer(Name);
        std::cout << SystemClockTimer::CurrentTimeString() << " - Starting build of: " << Name << std::endl;

        // CHECK IF ANY BUILDING NEEDS TO BE DONE.
        // Header-only libraries do not need any building.
        bool build_file_exists = !UnityBuildFilepath.empty();
        if (!build_file_exists)
        {
            std::cout << SystemClockTimer::CurrentTimeString() << " - Nothing to build for: " << Name << std::endl;
            return;
        }

        // DEFINE COMMON COMPILER OPTIONS.
        std::vector<std::string> common_compiler_options = 
        {
            "cl.exe",
            "/EHsc",
            /// @todo   Allow flag customization. "/WX",
            "/W4",
            "/TP",
            "/std:c++latest",
            /// @todo   Separate out debug/release options.
            "/Z7",
            "/Od",
            "/MTd"
        };

        // FORM THE COMMAND FOR COMPILING THE PROJECT.
        Command compilation_command = 
        {
            .Components = common_compiler_options
        };
        compilation_command.Components.push_back(UnityBuildFilepath.string());

        std::filesystem::path output_filepath = build_folder_path / Name;
        compilation_command.Components.push_back("/Fo:" + output_filepath.string());
        compilation_command.Components.push_back("/Fd:" + output_filepath.string());
        if (ProjectType::PROGRAM == Type)
        {
            compilation_command.Components.push_back("/Fe:" + output_filepath.string());
        }

        compilation_command.Components.push_back("/I");
        compilation_command.Components.push_back(CodeFolderPath.string());
        compilation_command.Components.push_back("/I");
        compilation_command.Components.push_back(CodeFolderPath.parent_path().string());

        for (const std::filesystem::path& include_path : AdditionalIncludeFolderPaths)
        {
            compilation_command.Components.push_back("/I");
            compilation_command.Components.push_back(include_path.string());
        }

        AddIncludeFolderPaths(compilation_command.Components);

        if (ProjectType::PROGRAM == Type)
        {
            std::vector<std::string> linker_library_inputs = GetLinkerLibraryInputs();
            if (!linker_library_inputs.empty())
            {
                compilation_command.Components.push_back("/link");
                for (const std::string& library_input : linker_library_inputs)
                {
                    compilation_command.Components.push_back(library_input);
                }

                compilation_command.Components.push_back("/LIBPATH:" + build_folder_path.string());
                AddLinkerFolderPaths(compilation_command.Components);
            }
        }

        compilation_command.Execute();

        if (ProjectType::LIBRARY == Type)
        {
            std::string obj_filename = Name + ".obj";
            std::filesystem::path output_obj_filepath = build_folder_path / obj_filename;
            Command create_library_command = 
            {
                .Components = 
                {
                    "lib.exe", output_obj_filepath.string()
                }
            };
            create_library_command.Execute();
        }
            
        std::cout 
            << SystemClockTimer::CurrentTimeString() 
            << " - Build of " << Name 
            << " finished"
            << " after " << command_timer.GetElapsedTimeText() << std::endl;
    }

    /// The type of the project.
    ProjectType Type = ProjectType::INVALID;
    /// A unique name identifying the project.
    std::string Name = "";
    /// The path to the folder containing all source code for the project.
    std::filesystem::path CodeFolderPath = "";
    /// The path to the unity (single translation unit) build file for the project.
    std::filesystem::path UnityBuildFilepath = "";
    /// Additional include directory paths.
    std::vector<std::filesystem::path> AdditionalIncludeFolderPaths = {};
    /// Libraries this project uses.
    std::vector<Project*> Libraries = {};
    /// Additional linker library names for the project.
    std::vector<std::string> LinkerLibraryNames = {};
};

/// The Microsoft Visual C++ compiler.
class VisualCppCompiler
{
public:
    /// Ensures the Visual C++ compiler is configured for x64 builds.
    static void ConfigureForX64()
    {
        // CHECK IF A VISUAL C++ INSTALL FOLDER HAS BEEN SET.
        const std::string VISUAL_CPP_INSTALL_FOLDER_ENVIRONMENT_VARIABLE_NAME = "VCINSTALLDIR";
        const char* visual_cpp_install_folder_path = std::getenv(VISUAL_CPP_INSTALL_FOLDER_ENVIRONMENT_VARIABLE_NAME.c_str());
        bool visual_cpp_install_folder_exists = (nullptr != visual_cpp_install_folder_path);

        // CHECK IF THE APPROPRIATE CPU ARCHITECTURE HAS BEEN SET.
        const std::string X64_CPU_ARCHITECTURE = "x64";
        const std::string VISUAL_CPP_TARGET_CPU_ARCHITECTURE_ENVIRONMENT_VARIABLE_NAME = "VSCMD_ARG_TGT_ARCH";
        const char* visual_cpp_target_cpu_architecture = std::getenv(VISUAL_CPP_TARGET_CPU_ARCHITECTURE_ENVIRONMENT_VARIABLE_NAME.c_str());
        bool x64_cpu_architecture_set = (nullptr != visual_cpp_target_cpu_architecture) && (X64_CPU_ARCHITECTURE == std::string(visual_cpp_target_cpu_architecture));

        // CHECK IF THE C++ COMPILER CAN BE FOUND.
        Command check_cpp_compiler_command = 
        {
            .Components = { "WHERE", "cl.exe" }
        };
        int check_cpp_compiler_command_return_code = check_cpp_compiler_command.Execute();
        bool cpp_compiler_in_path = (EXIT_SUCCESS == check_cpp_compiler_command_return_code);

        // ENSURE THE C++ COMPILER IS PROPERLY CONFIGURED.
        bool cpp_compiler_configured = (visual_cpp_install_folder_exists && x64_cpu_architecture_set && cpp_compiler_in_path);
        if (!cpp_compiler_configured)
        {
            // RUN A COMMAND TO CONFIGURE THE COMPILER.
            Command configure_cpp_compiler_command = 
            {
                .Components = { "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvarsall.bat", X64_CPU_ARCHITECTURE }
            };
            configure_cpp_compiler_command.Execute();
        }
    }
};

/// A build that can encompass multiple projects.
class Build
{
public:
    /// Adds a project to be built.
    /// @param[in]  project - The project to add.
    void Add(const Project& project)
    {
        Projects.push_back(project);
    }

    /// Runs the build to build all projects.
    void Run(const std::filesystem::path& workspace_folder_path)
    {
        // INDICATE THE BUILD IS STARTING.
        SystemClockTimer build_timer("Build");
        std::cout << SystemClockTimer::CurrentTimeString() << " - Starting build..." << std::endl;

        // ENSURE THE COMPILER IS PROPERLY CONFIGURED.
        /// @todo   Does not work for preserving environment on command line.  VisualCppCompiler::ConfigureForX64();

        // ENSURE THE BUILD FOLDER EXISTS.
        std::filesystem::path build_folder_path = workspace_folder_path / "build";
        std::filesystem::create_directories(build_folder_path);

        // BUILD EACH PROJECT.
        for (Project& project : Projects)
        {
            project.Build(build_folder_path);
        }

        // INDICATE THE BUILD HAS COMPLETED.
        std::cout 
            << SystemClockTimer::CurrentTimeString() 
            << " - Build complete after " << build_timer.GetElapsedTimeText()
            << std::endl;
    }

    /// The projects in the build.
    /// @todo   How to handle dependency ordering!
    std::vector<Project> Projects = {};
};
