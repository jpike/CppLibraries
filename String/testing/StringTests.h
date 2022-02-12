#pragma once

#include "String/String.hpp"

/// A namespace for testing the code in the corresponding class.
namespace STRING_TESTS
{
    TEST_CASE("A string can be split into lines.", "[String]")
    {
        const std::string MULTI_LINE_STRING = "1\n2\n3\n";
        std::vector<std::string> actual_lines = STRING::String::SplitIntoLines(MULTI_LINE_STRING);
        const std::vector<std::string> EXPECTED_LINES =
        {
            "1",
            "2",
            "3",
        };
        REQUIRE(EXPECTED_LINES == actual_lines);
    }

    TEST_CASE("A string can be split into words based on spaces.", "[String]")
    {
        const std::string MULTI_WORD_STRING = "ONE TWO  THREE";
        constexpr unsigned int MAX_LENGTH_PER_WORD_IN_CHARACTERS = 100;
        std::deque<std::string> actual_words = STRING::String::SplitIntoWords(MULTI_WORD_STRING, MAX_LENGTH_PER_WORD_IN_CHARACTERS);
        const std::deque<std::string> EXPECTED_WORDS =
        {
            "ONE",
            "TWO",
            "THREE",
        };
        REQUIRE(EXPECTED_WORDS == actual_words);
    }

    TEST_CASE("A string can be split into words based on a max length.", "[String]")
    {
        const std::string TEST_STRING = "WORLD";
        constexpr unsigned int MAX_LENGTH_PER_WORD_IN_CHARACTERS = 2;
        std::deque<std::string> actual_words = STRING::String::SplitIntoWords(TEST_STRING, MAX_LENGTH_PER_WORD_IN_CHARACTERS);
        const std::deque<std::string> EXPECTED_WORDS =
        {
            "WO",
            "RL",
            "D",
        };
        REQUIRE(EXPECTED_WORDS == actual_words);
    }
}
