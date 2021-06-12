#pragma once

#include <filesystem>
#include <optional>
#include <vector>
#include "Filesystem/File.h"

namespace FILESYSTEM
{
    /// A folder.
    class Folder
    {
    public:
        // CONSTRUCTION.
        static std::optional<Folder> GetIfExists(const std::filesystem::path& path);
        explicit Folder(const std::filesystem::path& path);

        // OTHER PUBLIC METHODS.
        std::vector<Folder> Subfolders() const;
        std::vector<File> Files() const;

        // MEMBER VARIABLES.
        /// The path to the folder.
        std::filesystem::path Path = "";
    };
}
