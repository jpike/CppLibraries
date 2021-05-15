#pragma once

#include <filesystem>
#include <string>

/// Holds code for interacting with computer filesystems.
namespace FILESYSTEM
{
    /// A single file containing data.
    class File
    {
    public:
        // STATIC METHODS.
        static std::string ReadBinary(const std::filesystem::path& path);
        // CONSTRUCTION.
        explicit File(const std::filesystem::path& path);

        // LINE COUNTING.
        unsigned long long CountLines() const;

        // MEMBER VARIABLES.
        /// The path to the file.
        std::filesystem::path Path = "";
    };
}
