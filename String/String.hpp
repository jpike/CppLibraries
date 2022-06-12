/// This filename ends with a .hpp extension to avoid conflicts with official C++ standard library "string" headers.

#pragma once

#include <deque>
#include <string>
#include <vector>

/// Holds code related to strings.
namespace STRING
{
    /// A class to hold utility methods related to strings.
    ///
    /// It is not intended to be treated like an instantiatable
    /// data type since a custom string class has not yet been
    /// necessary.
    class String
    {
    public:
        static std::vector<std::string> Split(const std::string& text, const char delimiter);

        static std::vector<std::string> SplitByWhitespace(const std::string& text);

        static std::vector<std::string> SplitIntoLines(const std::string& text);

        static std::deque<std::string> SplitIntoWords(
            const std::string& text,
            const unsigned int max_length_per_word_in_characters);
    };
}
