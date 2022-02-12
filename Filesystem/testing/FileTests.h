#pragma once

#include <filesystem>
#include "Filesystem/File.h"

/// A namespace for testing the code in the corresponding class.
namespace FILE_TESTS
{
    TEST_CASE("A binary file can be written and read.", "[File]")
    {
        // WRITE A BINARY FILE.
        const std::string BINARY_DATA = { 0x0A, 0x1B, 0x3C };
        const std::filesystem::path TEST_FILEPATH = "test.binaryfile";
        FILESYSTEM::File::WriteBinary(BINARY_DATA, TEST_FILEPATH);

        // READ THE BINARY FILE.
        std::string read_binary_data = FILESYSTEM::File::ReadBinary(TEST_FILEPATH);
        REQUIRE(BINARY_DATA == read_binary_data);

        // DELETE THE CREATED FILE TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove(TEST_FILEPATH);
    }

    TEST_CASE("Lines can be counted in a text file.", "[File]")
    {
        // WRITE A TEXT FILE.
        const std::string TEXT_LINES = "1\n2\n3\n";
        const std::filesystem::path TEST_FILEPATH = "test.txt";
        FILESYSTEM::File::WriteText(TEXT_LINES, TEST_FILEPATH);

        // COUNT LINES IN THE FILE.
        constexpr unsigned long long EXPECTED_LINE_COUNT = 3;
        FILESYSTEM::File test_file(TEST_FILEPATH);
        unsigned long long actual_line_count = test_file.CountLines();
        REQUIRE(EXPECTED_LINE_COUNT == actual_line_count);

        // DELETE THE CREATED FILE TO LEAVE THE FILESYSTEM IN A CLEAN STATE.
        std::filesystem::remove(TEST_FILEPATH);
    }
}
