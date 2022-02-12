#pragma once

#include <filesystem>
#include "Filesystem/DirectoryListing.h"

/// A namespace for testing the code in the corresponding class.
namespace DIRECTORY_LISTING_TESTS
{
    TEST_CASE("A directory listing one level deep can be printed.", "[DirectoryListing]")
    {
        // CREATE A FOLDER WITH A SUBFOLDER AND FILE.
        const std::filesystem::path ROOT_FOLDER_PATH = "TestFolder";
        std::filesystem::create_directories(ROOT_FOLDER_PATH);

        const std::filesystem::path SUBFOLDER_PATH = ROOT_FOLDER_PATH / "Subfolder";
        std::filesystem::create_directories(SUBFOLDER_PATH);

        const std::filesystem::path TEST_FILEPATH = ROOT_FOLDER_PATH / "TestFile.txt";
        FILESYSTEM::File::WriteText("Text", TEST_FILEPATH);

        // READ THE DIRECTORY LISTING.
        std::optional<FILESYSTEM::DirectoryListing> directory_listing = FILESYSTEM::DirectoryListing::Read(ROOT_FOLDER_PATH);

        // VERIFY THE DIRECTORY LISTING IS PROPERLY PRINTED.
        std::string actual_directory_listing_text = directory_listing->ToTextString();
        const std::string expected_directory_listing_text = R"("TestFolder"	[FOLDER]
	"TestFolder\\TestFile.txt"	[FILE]
	"TestFolder\\Subfolder"	[FOLDER]
)";
        REQUIRE(expected_directory_listing_text == actual_directory_listing_text);

        // DELETE THE FOLDER TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove_all(ROOT_FOLDER_PATH);
    }

    TEST_CASE("A directory listing two levels deep can be printed.", "[DirectoryListing]")
    {
        // CREATE A FOLDER WITH A SUBFOLDER AND FILE.
        const std::filesystem::path ROOT_FOLDER_PATH = "TestFolder";
        std::filesystem::create_directories(ROOT_FOLDER_PATH);

        const std::filesystem::path ROOT_TEST_FILEPATH = ROOT_FOLDER_PATH / "RootTestFile.txt";
        FILESYSTEM::File::WriteText("Root Text", ROOT_TEST_FILEPATH);

        const std::filesystem::path SUBFOLDER_PATH = ROOT_FOLDER_PATH / "Subfolder";
        std::filesystem::create_directories(SUBFOLDER_PATH);

        const std::filesystem::path SUBFOLDER_TEST_FILEPATH = SUBFOLDER_PATH / "SubfolderTestFile.txt";
        FILESYSTEM::File::WriteText("Subfolder Text", SUBFOLDER_TEST_FILEPATH);

        const std::filesystem::path SUB_SUBFOLDER_PATH = SUBFOLDER_PATH / "Subfolder2";
        std::filesystem::create_directories(SUB_SUBFOLDER_PATH);

        // READ THE DIRECTORY LISTING.
        std::optional<FILESYSTEM::DirectoryListing> directory_listing = FILESYSTEM::DirectoryListing::Read(ROOT_FOLDER_PATH);

        // VERIFY THE DIRECTORY LISTING IS PROPERLY PRINTED.
        std::string actual_directory_listing_text = directory_listing->ToTextString();
        const std::string expected_directory_listing_text = R"("TestFolder"	[FOLDER]
	"TestFolder\\RootTestFile.txt"	[FILE]
	"TestFolder\\Subfolder"	[FOLDER]
		"TestFolder\\Subfolder\\SubfolderTestFile.txt"	[FILE]
		"TestFolder\\Subfolder\\Subfolder2"	[FOLDER]
)";
        REQUIRE(expected_directory_listing_text == actual_directory_listing_text);

        // DELETE THE FOLDER TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove_all(ROOT_FOLDER_PATH);
    }
}
