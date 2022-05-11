#pragma once

/// Defines utilities for a simple C++ build system.  Currently only supports Windows.
///
/// The primary goals of this file is to provide a way to build multiple C++ projects
/// that may have dependencies on each other in a way that:
/// - Does not require installing a bunch of extra tools
///     (i.e. basically compiler + linker + what comes in the OS + this file)
/// - Allows taking advantage of features of a more sophisticated programming language
///     like C++ as opposed to batch/shell scripts.
///
/// Some minor enhancements for performance (faster builds) have been made using the C++
/// standard library's std::async() functionality.  However, only the simplest usage of
/// this functionality has been implemented - it's possible that further performance gains
/// could be achieved.  If parallel building is not working or not desirable, then
/// SERIAL_BUILD can be defined as a true value to revert back to serial builds.
///
/// This file was inspired by a variety of sources:
/// - After having wrestled with a lot of different build tools, I was pleased with the
///     relative simplicity of the single "batch" (.bat) file unity (single-translation-unit)
///     builds as used in places like Handmade Hero (https://handmadehero.org/), including
///     minimizing the additional stuff to install/learn.
/// - While .bat files are suitable for single projects, when trying to manage a bunch of them
///     for connecting multiple libraries, etc. together, becomes kind of unmaintainable,
///     especially for the awkard batch language (and shell/bash is not much better).
///     It would be better to leverage the abilities of more powerful languages - and namely
///     the same language as the core code being built to avoid having to learn some other
///     awkward language.  Newer programming languages like Jai, Zig, and Rust have features
///     like this.  Why not C++?
/// - Tricks like https://coderwall.com/p/e1htcg/self-compiling-source-code allow basically
///     having command-line-executable shell/batch scripts that can also contain C++ code.
///
/// While this file can be used however you want, it is primarily designed to be usable in
/// a "self-compiling" script such as a batch file that looks something like the following:
/// \code
/// #if 0
/// 
/// @ECHO OFF
/// 
/// REM MAKE SURE THE COMPILER IS IN THE PATH.
/// REM These build utilities expect these tools to already be in the PATH.
/// REM In the future, it might be possible to incorporate finding the compiler, etc.
/// REM in these build utilities.
/// REM Ex. Run vcvarsall.bat, etc. if needed.
/// 
/// REM BUILD THE C++ BUILD PROGRAM.
/// REM For exception handling flag - https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-160
/// REM /TP is needed to have this batch file treated as a .cpp file - https://docs.microsoft.com/en-us/cpp/build/reference/tc-tp-tc-tp-specify-source-file-type?view=msvc-160
/// cl.exe /std:c++latest /EHsc /TP build.bat
/// 
/// REM BUILD THE PROJECT
/// build.exe
/// 
/// @ECHO ON
/// EXIT /B
/// 
/// #endif
/// 
/// #include <filesystem>
/// #include "CppBuild.cpp"
/// 
/// int main()
/// {
///     // DEFINE THE PATH TO THE WORKSPACE.
///     // A "workspace" effectively defines the root folder that contains the codebase.
///     // It allows using relative paths that can be resolved from this root folder.
///     // The path is converted to an absolute path to ensure it remains correct in all commands.
///     std::filesystem::path workspace_folder_path = std::filesystem::absolute(".");
/// 
///     // DEFINE THE BUILD TO ADD PROJECTS TO.
///     Build build;
/// 
///     // DEFINE THE PROJECT TO BE BUILT.
///     Project example_library = 
///     {
///         .Type = ProjectType::LIBRARY,
///         .Name = "ExampleLibrary",
///         .CodeFolderPath = workspace_folder_path / "ExampleLibrary",
///         .UnityBuildFilepath = workspace_folder_path / "ExampleLibrary/ExampleLibrary.project"
///     };
///     build.Add(example_library);
///
///     // ... Additional projects can be added here ...
/// 
///     // RUN THE BUILD.
///     int build_exit_code = build.Run(workspace_folder_path);
///     return build_exit_code;
/// }
/// \endcode
///
/// This file is designed to be copied around and #included in your custom build scripts for ease-of-use
/// (hence a bunch of different classes directly in this file).  However, as you can see from the above, 
/// there are some limitations such as need explicit separate compilation/running of the "build" executable.
/// This will probably never be "perfect" since we have to live within the limitations that C++ gives us, 
/// but even with these limitations, this model has at least been better for me than alternatives.

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <future>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

/// A timer to allow timing execution of different parts of the build system.
template<typename ClockType = std::chrono::high_resolution_clock>
class Timer
{
public:
    /// Gets a string representing the current local time.
    /// @return A string for the current time.
    static std::string CurrentTimeString()
    {
        // GET THE CURRENT TIME.
        auto now = ClockType::to_time_t(ClockType::now());

        // CONVERT THE TIME TO A READABLE STRING.
        std::stringstream time_string_stream;
        time_string_stream << std::ctime(&now);
        std::string time_string = time_string_stream.str();
        // Remove annoying final newline characters.
        time_string.pop_back();
        return time_string;
    }

    /// Constructor to start the timer.
    explicit Timer() :
        StartTime(ClockType::now())
    {}

    /// Gets text describing the elapsed time.
    /// @return Text describing the elapsed time.
    std::string GetElapsedTimeText() const
    {
        // CALCULATE THE ELAPSED TIME.
        auto current_time = ClockType::now();
        auto elapsed_time = current_time - StartTime;

        // CONVERT THE ELAPSED TIME TO READABLE STRING.
        // Time is printed in several units to enable better assessing of performance and understanding of measurements.
        // Tabs separate the different unit-based printouts for easier readability.
        std::string elapsed_time_text = (
            std::to_string(elapsed_time.count()) + "\t" +
            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count()) + "ms\t" +
            std::to_string(std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count()) + "s");
        return elapsed_time_text;
    }

    /// The start time of the timer, according to the particular clock.
    std::chrono::time_point<ClockType> StartTime = {};
};

/// A timer using the highest resolution clock possible.
using HighResolutionTimer = Timer<std::chrono::high_resolution_clock>;
/// A timer using the system "wall clock time" clock.
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
        SystemClockTimer command_timer;
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
    /// Adds include folder paths for this project to the specified command line arguments.
    /// @param[in,out]  command_line_arguments - The command line arguments to add include path arguments to.
    void AddIncludeFolderPaths(std::vector<std::string>& command_line_arguments) const
    {
        // ENSURE THE PROJECT'S FILES CAN BE INCLUDED.
        command_line_arguments.push_back("/I");
        command_line_arguments.push_back(CodeFolderPath.string());
        // Parent folder paths are also added since projects will often have their names as the
            // code folder, and it is often desirable to have that name usable in #include statements.
        command_line_arguments.push_back("/I");
        command_line_arguments.push_back(CodeFolderPath.parent_path().string());

        // ENSURE ALL ADDITIONAL FILES CAN BE INCLUDED FROM EXPLICIT DIRECTORIES.
        for (const std::filesystem::path& include_path : AdditionalIncludeFolderPaths)
        {
            command_line_arguments.push_back("/I");
            command_line_arguments.push_back(include_path.string());
        }

        // ADD INCLUDE PATHS FROM ALL LIBRARIES.
        for (const Project* library : Libraries)
        {
            // ADD FOLDERS FROM THE DEPENDENCY.
            library->AddIncludeFolderPaths(command_line_arguments);
        }
    }

    /// Adds linker (lib) folder paths for this project to the specified command line arguments.
    /// @param[in,out]  command_line_arguments - The command line arguments to add linker path arguments to.
    void AddLinkerFolderPaths(std::vector<std::string>& command_line_arguments) const
    {
        // ADD EXPLICIT ADDITIONAL LIBRARY FOLDER PATHS.
        for (const std::filesystem::path& library_folder_path : AdditionalLibraryFolderPaths)
        {
            command_line_arguments.push_back("/LIBPATH:" + library_folder_path.string());
        }

        // ADD LINKER PATHS FROM ALL LIBRARIES.
        for (const Project* library : Libraries)
        {
            // ADD FOLDERS FROM THE IMMEDIATE LIBRARY.
            if (!library->CodeFolderPath.empty())
            {
                command_line_arguments.push_back("/LIBPATH:" + library->CodeFolderPath.string());
            }

            // ADD FOLDERS FROM ADDITIONAL DEPENDENCIES.
            library->AddLinkerFolderPaths(command_line_arguments);
        }
    }

    /// Gets all linker library inputs for this project.
    /// @return The names of linker library inputs (library files).
    std::vector<std::string> GetLinkerLibraryInputs() const
    {
        // INCLUDE EXPLICITLY SPECIFIED LIBRARY NAMES.
        std::vector<std::string> linker_library_names = LinkerLibraryNames;

        // ADD ALL LIBRARIES FROM DEPENDENCIES.
        for (const Project* library : Libraries)
        {
            // ADD ALL DEPENDENCIES FROM THE CURRENT LIBRARY.
            std::vector<std::string> dependency_libraries = library->GetLinkerLibraryInputs();
            for (const std::string& linker_input : dependency_libraries)
            {
                linker_library_names.push_back(linker_input);
            }
        }

        return linker_library_names;
    }

    /// Builds the project.
    /// @param[in]  build_root_folder_path - The path to root build folder in which to place outputs from building the project.
    ///     Build outputs for this project will be placed in a subfolder named for the build variant.
    /// @param[in]  build_variant - The variant to build (ex. "debug" or "release").  This affects build options and the output folder.
    /// @return A return code from building the project; returned to provide greater visibility into specific failures.
    int Build(const std::filesystem::path& build_root_folder_path, const std::string& build_variant)
    {
        // PROVIDE VISIBILITY INTO THIS PROJECT'S BUILD BEING STARTED.
        SystemClockTimer command_timer;
        std::cout << SystemClockTimer::CurrentTimeString() << " - Starting build of: " << Name << std::endl;

        // CHECK IF ANY BUILDING NEEDS TO BE DONE.
        // Header-only libraries do not need any building.
        bool build_file_exists = !UnityBuildFilepath.empty();
        if (!build_file_exists)
        {
            std::cout << SystemClockTimer::CurrentTimeString() << " - Nothing to build for: " << Name << std::endl;
            return EXIT_SUCCESS;
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
        };

        // ADD COMPILATION OPTIONS BASED ON THE BUILD VARIANT.
        bool is_release = ("release" == build_variant);
        if (is_release)
        {
            common_compiler_options.push_back("/O2");
            common_compiler_options.push_back("/MT");
        }
        else
        {
            // Assume a debug variant.
            common_compiler_options.push_back("/Z7");
            common_compiler_options.push_back("/Od");
            common_compiler_options.push_back("/MTd");
        }

        // ADD ANY CUSTOM COMPILER FLAGS.
        for (const std::string& compiler_option : CustomCompilerFlags)
        {
            common_compiler_options.push_back(compiler_option);
        }

        // FORM THE COMMAND FOR COMPILING THE PROJECT.
        Command compilation_command = 
        {
            .Components = common_compiler_options
        };
        compilation_command.Components.push_back(UnityBuildFilepath.string());

        // Ensure all output files are named based on the project.
        std::filesystem::path build_variant_output_path = build_root_folder_path / build_variant;
        std::filesystem::path output_filepath = build_variant_output_path / Name;
        compilation_command.Components.push_back("/Fo:" + output_filepath.string());
        compilation_command.Components.push_back("/Fd:" + output_filepath.string());
        if (ProjectType::PROGRAM == Type)
        {
            compilation_command.Components.push_back("/Fe:" + output_filepath.string());
        }

        // Ensure all appropriate files can be included.
        AddIncludeFolderPaths(compilation_command.Components);

        // Additional options may be needed based on type of project.
        if (ProjectType::LIBRARY == Type)
        {
            // Compile-only, without linking.
            // Needed to avoid "fatal error LNK1561: entry point must be defined" and a non-zero return code
            // that would cause builds to fail.
            compilation_command.Components.push_back("/c");
        }
        else if (ProjectType::PROGRAM == Type)
        {
            // ADD ALL LINKER INPUTS.
            std::vector<std::string> linker_library_inputs = GetLinkerLibraryInputs();
            if (!linker_library_inputs.empty())
            {
                compilation_command.Components.push_back("/link");
                for (const std::string& library_input : linker_library_inputs)
                {
                    compilation_command.Components.push_back(library_input);
                }

                // Appropriate library paths should also be added.
                compilation_command.Components.push_back("/LIBPATH:" + build_variant_output_path.string());
                AddLinkerFolderPaths(compilation_command.Components);
            }
        }

        // COMPILE THE PROJECT.
        int compilation_command_return_code = compilation_command.Execute();
        bool compilation_command_succeeded = (EXIT_SUCCESS == compilation_command_return_code);
        if (!compilation_command_succeeded)
        {
            return compilation_command_return_code;
        }

        // CREATE A LIBRARY FILE IF APPLICABLE.
        int build_return_code = compilation_command_return_code;
        if (ProjectType::LIBRARY == Type)
        {
            std::string obj_filename = Name + ".obj";
            std::filesystem::path output_obj_filepath = build_variant_output_path / obj_filename;
            Command create_library_command = 
            {
                .Components = 
                {
                    "lib.exe", output_obj_filepath.string()
                }
            };
            build_return_code = create_library_command.Execute();
        }
            
        // COMMUNICATE THE RESULTS OF THE BUILD.
        std::cout 
            << SystemClockTimer::CurrentTimeString() 
            << " - Build of " << Name 
            << " finished with return code " << build_return_code
            << " after " << command_timer.GetElapsedTimeText() << std::endl;
        return build_return_code;
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
    /// Additional library directory paths.
    std::vector<std::filesystem::path> AdditionalLibraryFolderPaths = {};
    /// Libraries this project uses.
    std::vector<Project*> Libraries = {};
    /// Additional linker library names for the project.
    std::vector<std::string> LinkerLibraryNames = {};
    /// Custom compiler flags for the project.
    std::vector<std::string> CustomCompilerFlags = {};
};

/// A task for building a single project.
class BuildTask
{
public:
    /// Creates a build task for the project.
    /// @param[in]  project - The project to build.
    /// @param[in]  build_folder_path - The path to the root build folder under which to put build outputs.
    /// @param[in]  build_variant - The variant to build (ex. "debug" or "release").  This affects build options and the output folder.
    static BuildTask Create(Project* project, const std::filesystem::path& build_folder_path, const std::string& build_variant)
    {
        BuildTask build_task;
        build_task.Project = project;

        // START THE TASK FOR BUILDING THE PROJECT.
        build_task.ReturnCodeBeingWaitedOn = std::async(
            std::launch::async,
            // It is important that these parameters be captured by value as the memory for parameter will differ later.
            [project, build_folder_path, build_variant]() -> int
            {
                return project->Build(build_folder_path, build_variant);
            });

        return build_task;
    }

    /// The project being built.
    Project* Project = nullptr;
    /// The future result of the build task.
    std::future<int> ReturnCodeBeingWaitedOn = {};
    /// The actual return code, once the task has completed.
    std::optional<int> ReturnCode = std::nullopt;
};

/// A build that can encompass multiple projects.
class Build
{
public:
    /// Adds a project to be built.
    /// @param[in]  project - The project to add.
    void Add(Project* project)
    {
        Projects.push_back(project);
    }

    /// Runs the build to build all projects.
    /// @param[in]  workspace_folder_path - The root folder for the workspace in which the build is occurring.
    /// @param[in]  build_variant - The variant to build (ex. "debug" or "release").  This affects build options and the output folder.
    /// @return A return code from the build.  This will generally be a code from build commands executed.
    int Run(const std::filesystem::path& workspace_folder_path, const std::string& build_variant)
    {
        // INDICATE THE BUILD IS STARTING.
        SystemClockTimer build_timer;
        std::cout << SystemClockTimer::CurrentTimeString() << " - Starting " << build_variant << " build..." << std::endl;

        // ENSURE THE BUILD VARIANT FOLDER EXISTS.
        std::filesystem::path build_folder_path = workspace_folder_path / "build";
        std::filesystem::path build_variant_folder_output_path = build_folder_path / build_variant;
        std::filesystem::create_directories(build_variant_folder_output_path);

        // PREPARE FOR BUILDING ALL PROJECTS.
        // These variables are needed for reporting final results.
        bool all_projects_built_successfully = true;
        int last_project_return_code = EXIT_SUCCESS;

#if SERIAL_BUILD
        // BUILD EACH PROJECT.
        for (Project* project : Projects)
        {
            // BUILD THE CURRENT PROJECT.
            last_project_return_code = project->Build(build_folder_path, build_variant);
            bool project_build_succeeded = (EXIT_SUCCESS == last_project_return_code);
            if (!project_build_succeeded)
            {
                // STOP EARLY TO PROVIDE EASIER VISIBILITY INTO THE FAILURE.
                all_projects_built_successfully = false;
                break;
            }
        }
#else
        // PROVIDE VISIBILITY INTO THE NUMBER OF THREADS SUPPORTED.
        // While the number of threads won't be explicitly used when spawning parallel build tasks,
        // this is printed to just provide general insight.
        unsigned int supported_thread_count = std::thread::hardware_concurrency();
        std::cout << supported_thread_count << " threads supported." << std::endl;

        // KICK OF BUILDS OF ANY PROJECTS WITHOUT ANY DEPENDENCIES.
        std::cout << "Starting to build projects without dependencies..." << std::endl;
        std::unordered_map<Project*, BuildTask> in_progress_build_tasks_by_project;
        std::vector<Project*> remaining_projects_to_build;
        for (Project* project : Projects)
        {
            // DETERMINE IF THE PROJECT HAS DEPENDENCIES.
            bool project_has_dependencies = !project->Libraries.empty();
            if (project_has_dependencies)
            {
                // TRACK THE PROJECT AS NEEDING TO BE BUILT LATER.
                remaining_projects_to_build.emplace_back(project);
            }
            else
            {
                // GO AHEAD AND START BUILDING THE PROJECT IF IT HAS NO DEPENDENCIES
                BuildTask build_task = BuildTask::Create(project, build_folder_path, build_variant);
                in_progress_build_tasks_by_project[project] = std::move(build_task);
            }
        }

        // FINISH KICKING OF BUILD OF ANY REMAINING PROJECTS.
        std::cout << "Starting to build remaining projects with dependencies..." << std::endl;
        std::unordered_map<Project*, BuildTask> completed_build_tasks_by_project;
        while (!remaining_projects_to_build.empty())
        {
            // CHECK FOR ANY COMPLETED BUILD TASKS.
            for (auto project_with_build_task = in_progress_build_tasks_by_project.begin(); project_with_build_task != in_progress_build_tasks_by_project.end(); )
            {
                // CHECK IF THE BUILD TASK HAS RECENTLY COMPLETED.
                bool build_task_recently_completed = project_with_build_task->second.ReturnCodeBeingWaitedOn.valid();
                if (build_task_recently_completed)
                {
                    // GET THE RETURN CODE FROM THE BUILD TASK.
                    last_project_return_code = project_with_build_task->second.ReturnCodeBeingWaitedOn.get();
                    project_with_build_task->second.ReturnCode = last_project_return_code;

                    // MOVE THE BUILD TASK TO THE COMPLETED LIST.
                    completed_build_tasks_by_project[project_with_build_task->first] = std::move(project_with_build_task->second);

                    // PROGRESS TO THE NEXT IN-PROGRESS BUILD TASK.
                    project_with_build_task = in_progress_build_tasks_by_project.erase(project_with_build_task);

                    // CHECK IF THE BUILD TASK SUCCEEDED.
                    bool build_task_succeeded = (EXIT_SUCCESS == last_project_return_code);
                    if (!build_task_succeeded)
                    {
                        // CLEAR THE REMAINING PROJECTS TO BUILD TO EXIT OUT OF THE ABOVE LOOP EARLY.
                        // This allows simplifying error handling below.
                        remaining_projects_to_build.clear();
                        all_projects_built_successfully = false;
                        break;
                    }
                }
                else
                {
                    ++project_with_build_task;
                }
            }

            // KICK OFF BUILD TASKS FOR ANY REMAINING PROJECTS WHOSE DEPENDENCIES HAVE FINISHED BEING BUILT.
            for (auto remaining_project_to_build = remaining_projects_to_build.begin(); remaining_project_to_build != remaining_projects_to_build.end(); )
            {
                // CHECK IF ALL DEPENDENCIES FOR THE PROJECT HAVE BEEN BUILT.
                std::size_t built_dependency_count = 0;
                for (Project* dependency : (*remaining_project_to_build)->Libraries)
                {
                    // COUNT THE DEPENDENCY IF IT HAS FINISHED BEING BUILT SUCCESSFULLY.
                    bool dependency_build_task_completed = completed_build_tasks_by_project.contains(dependency);
                    if (dependency_build_task_completed)
                    {
                        BuildTask& dependency_build_task = completed_build_tasks_by_project[dependency];
                        bool dependency_built_successfully = (EXIT_SUCCESS == dependency_build_task.ReturnCode);
                        if (dependency_built_successfully)
                        {
                            ++built_dependency_count;
                        }
                    }
                }

                // START BUILDING THE CURRENT PROJECT IF ALL DEPENDENCIES HAVE BEEN BUILT.
                std::size_t total_dependency_count = (*remaining_project_to_build)->Libraries.size();
                bool all_dependencies_built = (built_dependency_count >= total_dependency_count);
                if (all_dependencies_built)
                {
                    // START BUILDING THIS PROJECT.
                    BuildTask build_task = BuildTask::Create(*remaining_project_to_build, build_folder_path, build_variant);
                    in_progress_build_tasks_by_project[*remaining_project_to_build] = std::move(build_task);

                    // REMOVE THE PROJECT AS NEEDING TO BE BUILT.
                    remaining_project_to_build = remaining_projects_to_build.erase(remaining_project_to_build);
                }
                else
                {
                    // MOVE TO CHECKING THE NEXT PROJECT.
                    ++remaining_project_to_build;
                }
            }
        }

        // WAIT FOR ALL BUILD TASKS TO COMPLETE.
        std::cout << "Waiting on all build tasks to complete..." << std::endl;
        while (!in_progress_build_tasks_by_project.empty())
        {
            // CHECK FOR ANY COMPLETED BUILD TASKS.
            for (auto project_with_build_task = in_progress_build_tasks_by_project.begin(); project_with_build_task != in_progress_build_tasks_by_project.end(); )
            {
                // CHECK IF THE BUILD TASK HAS RECENTLY COMPLETED.
                bool build_task_recently_completed = project_with_build_task->second.ReturnCodeBeingWaitedOn.valid();
                if (build_task_recently_completed)
                {
                    // GET THE RETURN CODE FROM THE BUILD TASK.
                    last_project_return_code = project_with_build_task->second.ReturnCodeBeingWaitedOn.get();
                    project_with_build_task->second.ReturnCode = last_project_return_code;

                    // MOVE THE BUILD TASK TO THE COMPLETED LIST.
                    completed_build_tasks_by_project[project_with_build_task->first] = std::move(project_with_build_task->second);

                    // PROGRESS TO THE NEXT IN-PROGRESS BUILD TASK.
                    project_with_build_task = in_progress_build_tasks_by_project.erase(project_with_build_task);

                    // CHECK IF THE BUILD TASK SUCCEEDED.
                    bool build_task_succeeded = (EXIT_SUCCESS == last_project_return_code);
                    if (!build_task_succeeded)
                    {
                        // CLEAR THE REMAINING PROJECTS TO BUILD TO EXIT OUT OF THE ABOVE LOOP EARLY.
                        // This allows simplifying error handling below.
                        in_progress_build_tasks_by_project.clear();
                        all_projects_built_successfully = false;
                        break;
                    }
                }
                else
                {
                    ++project_with_build_task;
                }
            }
        }
#endif

        // INDICATE THE RESULT OF THE BUILD.
        if (all_projects_built_successfully)
        {
            std::cout
                << SystemClockTimer::CurrentTimeString()
                << " - Build (" << build_variant << ") completed successfully after " << build_timer.GetElapsedTimeText()
                << std::endl;
        }
        else
        {
            std::cout
                << SystemClockTimer::CurrentTimeString()
                << " - Build (" << build_variant << ") failed after " << build_timer.GetElapsedTimeText()
                << std::endl;
        }

        return last_project_return_code;
    }

    /// The projects in the build.
    /// @todo   How to handle dependency ordering!
    std::vector<Project*> Projects = {};
};
