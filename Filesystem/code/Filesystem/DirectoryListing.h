#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include "Filesystem/File.h"
#include "Filesystem/Folder.h"

namespace FILESYSTEM
{
    /// A listing of files in a directory.
    class DirectoryListing
    {
    public:
        // CONSTRUCTION.
        static std::optional<DirectoryListing> Read(const std::filesystem::path& root_path);
        explicit DirectoryListing(const Folder& root_folder);

        // PRINTING.
        std::string ToTextString(const std::size_t indentation_level = 0) const;

        // MEMBER VARIABLES.
        Folder RootFolder;
        std::vector<DirectoryListing> Subfolders = {};
        std::vector<File> Files = {};
    };
}
