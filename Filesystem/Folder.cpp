#include "Filesystem/Folder.h"

namespace FILESYSTEM
{
    /// Gets the folder if it actually exists on the filesystem.
    /// @param[in]  path - The path of the folder to get.
    /// @return The folder, if successfully found.
    std::optional<Folder> Folder::GetIfExists(const std::filesystem::path& path)
    {
        // GET THE FOLDER IF IT EXISTS.
        bool folder_exists = std::filesystem::is_directory(path);
        if (folder_exists)
        {
            return Folder(path);
        }
        else
        {
            return std::nullopt;
        }
    }

    /// Constructor.
    /// @param[in]  path - THe path of the folder.  Assumed to be valid.
    Folder::Folder(const std::filesystem::path& path) :
        Path(path)
    {}

    /// Gets all direct subfolders of this folder.
    /// @return All direct subfolders.
    std::vector<Folder> Folder::Subfolders() const
    {
        std::vector<Folder> subfolders;

        // GET ALL SUBFOLDERS WITHIN THIS FOLDER.
        for (const auto& directory_entry : std::filesystem::directory_iterator(Path))
        {
            // ONLY GET THE CURRENT DIRECTORY ENTRY IF IT'S A FOLDER.
            bool entry_for_folder = directory_entry.is_directory();
            if (entry_for_folder)
            {
                subfolders.emplace_back(directory_entry.path());
            }
        }

        return subfolders;
    }

    /// Gets all files directly within this folder.
    /// @return All files directly within this folder.
    std::vector<File> Folder::Files() const
    {
        std::vector<File> files;

        // GET ALL DIRECT FILES WITHIN THIS FOLDER.
        for (const auto& directory_entry : std::filesystem::directory_iterator(Path))
        {
            // ONLY GET THE CURRENT DIRECTORY ENTRY IF IT'S A FILE.
            bool entry_for_file = directory_entry.is_regular_file();
            if (entry_for_file)
            {
                files.emplace_back(directory_entry.path());
            }
        }

        return files;
    }
}