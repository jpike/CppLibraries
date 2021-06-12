#include <sstream>
#include "Filesystem/DirectoryListing.h"

namespace FILESYSTEM
{
    /// Reads a directory listing from the filesystem for the specified path.
    /// The listing is fully recursive.
    /// @param[in]  root_path - The path to the root folder for which to read a directory listing.
    /// @return The directory listing, if successfully retrieved.
    std::optional<DirectoryListing> DirectoryListing::Read(const std::filesystem::path& root_path)
    {
        // GET THE ROOT FOLDER IF IT EXISTS.
        std::optional<Folder> root_folder = Folder::GetIfExists(root_path);
        if (!root_folder)
        {
            return std::nullopt;
        }

        // CREATE THE INITIAL DIRECTORY LISTING.
        DirectoryListing directory_listing(*root_folder);

        // POPULATE THE CONTENTS OF THE DIRECTORY LISTING.
        for (const auto& directory_entry : std::filesystem::directory_iterator(directory_listing.RootFolder.Path))
        {
            // CHECK IF THE CURRENT ENTRY IS FOR A FOLDER OR FILE.
            if (directory_entry.is_directory())
            {
                // GET THE DIRECTORY LISTING FOR THE SUBFOLDER.
                std::optional<DirectoryListing> subfolder_listing = DirectoryListing::Read(directory_entry);
                if (subfolder_listing)
                {
                    directory_listing.Subfolders.push_back(*subfolder_listing);
                }
                else
                {
                    // An error occurred getting the full directory listing.
                    return std::nullopt;
                }
            }
            else if (directory_entry.is_regular_file())
            {
                // ADD THE FILE TO THE DIRECTORY LISTING.
                directory_listing.Files.emplace_back(directory_entry);
            }
            else
            {
                // SKIP OVER THIS FILE SINCE WE'RE NOT SURE WHAT IT'S FOR.
                continue;
            }
        }

        return directory_listing;
    }

    /// Constructor.
    /// @param[in]  root_folder - The root folder for the directory listing.
    DirectoryListing::DirectoryListing(const Folder& root_folder) :
        RootFolder(root_folder),
        Subfolders(),
        Files()
    {}

    /// Converts the directory listing to a textual string.
    /// @param[in]  indentation_level - The indentation level to use for the root folder of this
    ///     directory listing.  Entries deeper in the directory listing are indented further.
    /// @return A string representation of the directory listing.
    std::string DirectoryListing::ToTextString(const std::size_t indentation_level) const
    {
        std::ostringstream directory_listing_text;

        // PRINT THE CURRENT DIRECTORY PATH.
        const char INDENTATION_CHARACTER = '\t';
        std::string indentation(indentation_level, INDENTATION_CHARACTER);
        const std::string FOLDER_INDICATOR = "[FOLDER]";
        directory_listing_text << indentation << RootFolder.Path << INDENTATION_CHARACTER << FOLDER_INDICATOR << std::endl;

        // PRINT ALL CHILD FILES.
        for (const auto& child_file : Files)
        {
            std::size_t child_indentation_level = indentation_level + 1;
            std::string child_indentation(child_indentation_level, INDENTATION_CHARACTER);
            const std::string FILE_INDICATOR = "[FILE]";
            directory_listing_text << child_indentation << child_file.Path << INDENTATION_CHARACTER << FILE_INDICATOR << std::endl;
        }

        // PRINT ALL CHILD FOLDERS.
        for (const auto& child_directory_listing : Subfolders)
        {
            std::size_t child_indentation_level = indentation_level + 1;
            directory_listing_text << child_directory_listing.ToTextString(child_indentation_level);
        }

        return directory_listing_text.str();
    }
}
