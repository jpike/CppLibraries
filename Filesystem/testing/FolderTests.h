#pragma once

#include <filesystem>
#include "Filesystem/Folder.h"

/// A namespace for testing the code in the corresponding class.
namespace FOLDER_TESTS
{
    TEST_CASE("A non-existent folder cannot be obtained.", "[Folder]")
    {
        std::optional<FILESYSTEM::Folder> folder = FILESYSTEM::Folder::GetIfExists("NonExistentFolder");
        REQUIRE(!folder.has_value());
    }

    TEST_CASE("A existing folder can be obtained.", "[Folder]")
    {
        // CREATE A FOLDER.
        const std::filesystem::path FOLDER_PATH = "TestFolder";
        std::filesystem::create_directories(FOLDER_PATH);

        // MAKE SURE THE FOLDER CAN BE OBTAINED.
        std::optional<FILESYSTEM::Folder> folder = FILESYSTEM::Folder::GetIfExists(FOLDER_PATH);
        REQUIRE(FOLDER_PATH == folder->Path);

        // DELETE THE FOLDER TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove_all(FOLDER_PATH);
    }

    TEST_CASE("Subfolders wtihin a folder can be obtained.", "[Folder]")
    {
        // CREATE A FOLDER WITH SUBFOLDERS.
        const std::filesystem::path ROOT_FOLDER_PATH = "TestFolder";
        std::filesystem::create_directories(ROOT_FOLDER_PATH);

        const std::filesystem::path SUBFOLDER_PATH_1 = ROOT_FOLDER_PATH / "Subfolder1";
        std::filesystem::create_directories(SUBFOLDER_PATH_1);

        const std::filesystem::path SUBFOLDER_PATH_2 = ROOT_FOLDER_PATH / "Subfolder2";
        std::filesystem::create_directories(SUBFOLDER_PATH_2);

        // GET SUBFOLDERS FROM THE FOLDER.
        FILESYSTEM::Folder root_folder(ROOT_FOLDER_PATH);
        std::vector<FILESYSTEM::Folder> actual_subfolders = root_folder.Subfolders();

        // MAKE SURE THE CORRECT SUBFOLDERS WERE OBTAINED.
        std::vector<FILESYSTEM::Folder> expected_subfolders =
        {
            FILESYSTEM::Folder(SUBFOLDER_PATH_1),
            FILESYSTEM::Folder(SUBFOLDER_PATH_2),
        };
        REQUIRE(expected_subfolders == actual_subfolders);

        // DELETE THE FOLDER TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove_all(ROOT_FOLDER_PATH);
    }

    TEST_CASE("Files within a folder can be obtained.", "[Folder]")
    {
        // CREATE A FOLDER WITH FILES.
        const std::filesystem::path ROOT_FOLDER_PATH = "TestFolder";
        std::filesystem::create_directories(ROOT_FOLDER_PATH);

        const std::filesystem::path TEST_FILEPATH_1 = ROOT_FOLDER_PATH / "TestFile1.txt";
        FILESYSTEM::File::WriteText("Text 1", TEST_FILEPATH_1);

        const std::filesystem::path TEST_FILEPATH_2 = ROOT_FOLDER_PATH / "TestFile2.txt";
        FILESYSTEM::File::WriteText("Text 2", TEST_FILEPATH_2);
        
        // GET FILES FROM THE FOLDER.
        FILESYSTEM::Folder root_folder(ROOT_FOLDER_PATH);
        std::vector<FILESYSTEM::File> actual_files = root_folder.Files();

        // MAKE SURE THE CORRECT FILES WERE OBTAINED.
        std::vector<FILESYSTEM::File> expected_files =
        {
            FILESYSTEM::File(TEST_FILEPATH_1),
            FILESYSTEM::File(TEST_FILEPATH_2),
        };
        REQUIRE(expected_files == actual_files);

        // DELETE THE FOLDER TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove_all(ROOT_FOLDER_PATH);
    }
}
