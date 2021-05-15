#include <cctype>
#include "String/String.h"

namespace STRING
{
    /// Splits the provided string into separate lines based on newline
    /// characters embedded in the string.  Only '\n' is currently supported
    /// since that is all tha is currently needed.
    /// @param[in]  text - The text to split into lines.
    /// @return The lines from the provided text, in original order.
    std::vector<std::string> String::SplitIntoLines(const std::string& text)
    {
        std::vector<std::string> lines;

        // EXTRACT ALL LINES FROM THE TEXT.
        std::string current_line;
        for (const char character : text)
        {
            // CHECK IF THE CURRENT CHARACTER IS A NEWLINE.
            const char NEWLINE = '\n';
            bool end_of_line_reached = (NEWLINE == character);
            if (end_of_line_reached)
            {
                // ADD THE LINE FROM THE MOST RECENT SEQUENCE OF CHARACTERS IF ONE EXISTS.
                bool line_exists = !current_line.empty();
                if (line_exists)
                {
                    lines.push_back(current_line);

                    // To prepare for a new line, the current line must be cleared.
                    current_line.clear();
                }

                // CONTINUE TO THE NEXT CHARACTER.
                // A newline isn't considered part of the line itself and can be discarded.
                continue;
            }
            else
            {
                // ADD THE CURRENT CHARACTER TO THE CURRENT LINE.
                current_line += character;
            }
        }

        // ADD THE FINAL LINE IF ONE EXISTS.
        // If the above loop ended without a newline,
        // a line may still exist that needs to be added.
        bool final_line_exists = !current_line.empty();
        if (final_line_exists)
        {
            lines.push_back(current_line);
        }

        return lines;
    }

    /// Splits the provided string into separate words.
    /// Any whitespace character is considered a delimiter.
    /// Punctuation is included in adjacent words.
    /// @param[in]  text - The text to split into words.
    /// @param[in]  max_length_per_word_in_characters - The maximum length (in characters)
    ///     for each word.  If a word is found that exceeds this maximum length, it will
    ///     be broken up into separate words.
    /// @return The words from the provided text, in original order.
    std::deque<std::string> String::SplitIntoWords(
        const std::string& text,
        const unsigned int max_length_per_word_in_characters)
    {
        std::deque<std::string> words;

        // EXTRACT ALL WORDS FROM THE TEXT.
        std::string current_word;
        for (const char character : text)
        {
            // CHECK IF THE CURRENT CHARACTER IS A SPACE.
            // Spaces are the only characters that aren't considered part
            // of a word.  Additional punctuation in-between or around
            // a word will be considered part of that word since it typically
            // is desirable to display that punctuation without breaking
            // up the word.
            const int NON_SPACE = 0;
            bool is_space = (NON_SPACE != isspace(character));
            if (is_space)
            {
                // ADD THE WORD FROM THE MOST RECENT SEQUENCE OF CHARACTERS IF ONE EXISTS.
                bool word_exists = !current_word.empty();
                if (word_exists)
                {
                    words.push_back(current_word);

                    // To prepare for a new word, the current word must be cleared.
                    current_word.clear();
                }

                // CONTINUE TO THE NEXT CHARACTER.
                // A space isn't considered part of a word and can be discarded.
                continue;
            }
            else
            {
                // ADD THE CURRENT CHARACTER TO THE CURRENT WORD.
                current_word += character;

                // ADD THE WORD IF IT HAS REACHED THE MAXIMUM ALLOWED LENGTH.
                size_t current_word_length_in_characters = current_word.length();
                bool word_met_max_length = (current_word_length_in_characters >= max_length_per_word_in_characters);
                if (word_met_max_length)
                {
                    words.push_back(current_word);

                    // To prepare for a new word, the current word must be cleared.
                    current_word.clear();
                }
            }
        }

        // ADD THE FINAL WORD IF ONE EXISTS.
        // If the above loop ended without a space or a word meeting the max length,
        // a word may still exist that needs to be added.
        bool final_word_exists = !current_word.empty();
        if (final_word_exists)
        {
            words.push_back(current_word);
        }

        return words;
    }
}
