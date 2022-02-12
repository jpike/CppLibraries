#pragma once

#include <compare>
#include <filesystem>
#include <string>
#include <string_view>

/// Holds code for interacting with computer filesystems.
namespace FILESYSTEM
{
    /// A single file containing data.
    class File
    {
    public:
        // STATIC METHODS.
        static std::string ReadBinary(const std::filesystem::path& path);
        static void WriteBinary(const std::string_view binary_data, const std::filesystem::path& path);
        static void WriteText(const std::string_view text, const std::filesystem::path& path);
        // CONSTRUCTION.
        explicit File(const std::filesystem::path& path);

        // OPERATORS.
        auto operator<=>(const File&) const = default;

        // LINE COUNTING.
        unsigned long long CountLines() const;

        // MEMBER VARIABLES.
        /// The path to the file.
        std::filesystem::path Path = "";
    };
}
