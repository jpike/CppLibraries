#include <fstream>
#include "Filesystem/File.h"

namespace FILESYSTEM
{
    /// Attempts to read all data in binary format from the specified file.
    /// @param[in]  path - The path of the file to read.
    /// @return All binary data from the file, if successfully read; empty otherwise.
    std::string File::ReadBinary(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::in);
        auto beginning_of_file = std::istreambuf_iterator<char>(file);
        auto end_of_file = std::istreambuf_iterator<char>();
        std::string binary_data(beginning_of_file, end_of_file);
        return binary_data;
    }

    /// Writes a binary file.
    /// @param[in]  binary_data - The binary data to write.
    /// @param[in]  path - The path of the file to write.
    void File::WriteBinary(const std::string_view binary_data, const std::filesystem::path& path)
    {
        std::ofstream binary_file(path, std::ios::binary);
        binary_file.write(binary_data.data(), binary_data.size());
    }

    /// Writes a text file.
    /// @param[in]  text - The text to write.
    /// @param[in]  path - The path of the file to write.
    void File::WriteText(const std::string_view text, const std::filesystem::path& path)
    {
        std::ofstream text_file(path);
        text_file.write(text.data(), text.size());
    }
    
    /// Constructor.
    /// @param[in]  path - The path to the file.
    File::File(const std::filesystem::path& path) :
        Path(path)
    {}

    /// Counts the number of lines in the file.
    /// @return The number of lines in the file.
    unsigned long long File::CountLines() const
    {
        // OPEN THE FILE.
        std::ifstream file(Path);

        // COUNT EACH LINE.
        unsigned long long line_count = 0;
        std::string line;
        while (std::getline(file, line))
        {
            ++line_count;
        }

        return line_count;
    }
}
