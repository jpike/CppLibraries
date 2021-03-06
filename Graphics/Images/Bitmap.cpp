#include <fstream>
#if _WIN32
#include <Windows.h>
#endif
#include "stb/stb_image.h"
#include "Graphics/Images/Bitmap.h"

namespace GRAPHICS::IMAGES
{
#if _WIN32
    /// Attempts to load the bitmap from the specified filepath.
    /// @param[in]  filepath - The path to the bitmap file to load.
    /// @return The texture, if loaded successfully; null otherwise.
    std::shared_ptr<Bitmap> Bitmap::Load(const std::filesystem::path& filepath)
    {
        // OPEN THE FILE.
        std::ifstream bitmap_file(filepath, std::ios::binary);
        bool bitmap_file_opened = bitmap_file.is_open();
        if (!bitmap_file_opened)
        {
            return nullptr;
        }

        // See https://en.wikipedia.org/wiki/BMP_file_format for the .bmp file format.
        // https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-storage

        // READ IN THE BITMAP FILE HEADER.
        BITMAPFILEHEADER bitmap_file_header = {};
        bitmap_file.read(reinterpret_cast<char*>(&bitmap_file_header), sizeof(bitmap_file_header));

        // READ IN THE BITMAP INFO HEADER.
        // This is assumed to be next based on our specific bitmaps, but it's not fully robust
        // against all bitmaps.
        BITMAPINFOHEADER bitmap_info_header = {};
        bitmap_file.read(reinterpret_cast<char*>(&bitmap_info_header), sizeof(bitmap_info_header));

        // CREATE THE EMPTY BITMAP.
        auto bitmap = std::make_shared<Bitmap>(
            bitmap_info_header.biWidth,
            bitmap_info_header.biHeight,
            ColorFormat::RGBA); /// Seems to be RGB based on biCompression == 0

        // FILL IN ALL PIXELS OF THE BITMAP.
        // When height is non-negative, then this is a bottom-up bitmap - hence why we
        // start at the last row of pixels.
        unsigned int current_pixel_x = 0;
        unsigned int current_pixel_y = bitmap_info_header.biHeight - 1;
        while (bitmap_file.good())
        {
            // READ IN THE CURRENT PIXEL'S COLOR COMPONENTS.
            // See https://docs.microsoft.com/en-us/previous-versions//dd183376(v=vs.85)?redirectedfrom=MSDN
            // about pixel color component ordering.
            uint8_t blue = 0;
            bitmap_file.read(reinterpret_cast<char*>(&blue), sizeof(blue));

            uint8_t green = 0;
            bitmap_file.read(reinterpret_cast<char*>(&green), sizeof(green));

            uint8_t red = 0;
            bitmap_file.read(reinterpret_cast<char*>(&red), sizeof(red));

            // If reading failed, then the above data isn't valid.
            if (!bitmap_file.good())
            {
                break;
            }

            // SET THE COLOR IN THE BITMAP.
            constexpr uint8_t ALPHA_FOR_FULLY_OPAQUE = 1;
            Color color(red, green, blue, ALPHA_FOR_FULLY_OPAQUE);
            bitmap->WritePixel(current_pixel_x, current_pixel_y, color);

            // MOVE TO THE NEXT PIXEL.
            ++current_pixel_x;
            bool current_row_filled = (current_pixel_x >= static_cast<unsigned int>(bitmap_info_header.biWidth));
            if (current_row_filled)
            {
                // MOVE TO THE NEXT ROW.
                current_pixel_x = 0;
                --current_pixel_y;
            }
        }

        return bitmap;
    }
#endif

    /// Attempts to load a bitmap from a PNG file at the specified filepath.
    /// @param[in]  filepath - The path to the PNG file to load as a bitmap.
    /// @param[in]  color_format - The color format to use for the resulting image.
    /// @return The bitmap, if loaded successfully; null otherwise.
    std::shared_ptr<Bitmap> Bitmap::LoadPng(const std::filesystem::path& filepath, const GRAPHICS::ColorFormat color_format)
    {
        // LOAD THE IMAGE DATA FROM FILE.
        int width_in_pixels = 0;
        int height_in_pixels = 0;
        int actual_pixel_component_count = 0;
        // 4 components per-pixel are needed for compatibility with our supported color formats.
        constexpr int REQUIRED_PIXEL_COMPONENT_COUNT = 4;
        uint8_t* pixels = stbi_load(
            filepath.string().c_str(), 
            &width_in_pixels, 
            &height_in_pixels, 
            &actual_pixel_component_count, 
            REQUIRED_PIXEL_COMPONENT_COUNT);
        if (pixels)
        {
            // CONVERT THE PIXELS TO A PROPER BITMAP.
            auto bitmap = std::make_shared<Bitmap>(
                static_cast<unsigned int>(width_in_pixels),
                static_cast<unsigned int>(height_in_pixels),
                color_format);

            for (int pixel_y = 0; pixel_y < height_in_pixels; ++pixel_y)
            {
                // CALCULATE THE INDEX OF THE FIRST ELEMENT IN THE REQUESTED ROW.
                int pixel_row_index = pixel_y * width_in_pixels;

                for (int pixel_x = 0; pixel_x < width_in_pixels; ++pixel_x)
                {
                    // MOVE OVER TO THE REQUESTED ELEMENT IN THE ROW.
                    int pixel_start_element_index = (pixel_row_index + pixel_x) * REQUIRED_PIXEL_COMPONENT_COUNT;

                    // FORM THE CURRENT COLOR.
                    int current_pixel_red_component_index = pixel_start_element_index;
                    uint8_t red = pixels[current_pixel_red_component_index];

                    int current_pixel_green_component_index = current_pixel_red_component_index + 1;
                    uint8_t green = pixels[current_pixel_green_component_index];

                    int current_pixel_blue_component_index = current_pixel_green_component_index + 1;
                    uint8_t blue = pixels[current_pixel_blue_component_index];

                    int current_pixel_alpha_component_index = current_pixel_blue_component_index + 1;
                    uint8_t alpha = pixels[current_pixel_alpha_component_index];

                    Color color(red, green, blue, alpha);

                    // WRITE THE CURRENT COLOR TO THE BITMAP.
                    bitmap->WritePixel(pixel_x, pixel_y, color);
                }
            }

            // FREE THE MEMORY USED FOR THE RAW FILE DATA.
            stbi_image_free(pixels);

            return bitmap;
        }
        else
        {
            // INDICATE THE IMAGE COULD NOT BE LOADED.
            return nullptr;
        }
    }

    /// Constructor.
    /// @param[in]  width_in_pixels - The width of the bitmap.
    /// @param[in]  height_in_pixels - The height of the bitmap.
    /// @param[in]  color_format - The color format of pixels in the bitmap.
    Bitmap::Bitmap(
        const unsigned int width_in_pixels,
        const unsigned int height_in_pixels,
        const GRAPHICS::ColorFormat color_format) :
        WidthInPixels(width_in_pixels),
        HeightInPixels(height_in_pixels),
        ColorFormat(color_format),
        Pixels(width_in_pixels, height_in_pixels)
    {}

    /// Gets the width of the bitmap.
    /// @return The width in pixels.
    unsigned int Bitmap::GetWidthInPixels() const
    {
        return WidthInPixels;
    }

    /// Gets the height of the bitmap.
    /// @return The height in pixels.
    unsigned int Bitmap::GetHeightInPixels() const
    {
        return HeightInPixels;
    }

    /// Gets the number of bytes in a single row of the bitmap.
    /// Also known as the stride or pitch.
    /// @return The number of bytes per row in the bitmap.
    unsigned int Bitmap::GetRowByteCount() const
    {
        constexpr unsigned int PIXEL_BYTE_COUNT = sizeof(uint32_t);
        unsigned int row_byte_count = WidthInPixels * PIXEL_BYTE_COUNT;
        return row_byte_count;
    }

    /// Gets the color format of pixels in the bitmap.
    /// @return The color format of pixels in the bitmap.
    GRAPHICS::ColorFormat Bitmap::GetColorFormat() const
    {
        return ColorFormat;
    }

    /// Retrieves a pointer to the raw pixel data of the bitmap.
    /// @return A pointer to the raw pixel data.
    const uint32_t* Bitmap::GetRawData() const
    {
        return Pixels.ValuesInRowMajorOrder();
    }

    /// Retrieves a pointer to the raw pixel data of the bitmap.
    /// @return A pointer to the raw pixel data.
    uint32_t* Bitmap::GetRawData()
    {
        return Pixels.ValuesInRowMajorOrder();
    }

    /// Retrieves the pixel color at the specified coordinates.
    /// @param[in]  x - The horizontal coordinate of the pixel.
    /// @param[in]  y - The vertical coorindate of the pixel.
    GRAPHICS::Color Bitmap::GetPixel(const unsigned int x, const unsigned int y) const
    {
        // RETURN A DEFAULT COLOR IF THE PIXEL COORDINATES AREN'T VALID.
        bool pixel_coordinates_valid = Pixels.IndicesInRange(x, y);
        if (!pixel_coordinates_valid)
        {            
            return GRAPHICS::Color::BLACK;
        }

        // RETURN THE COLOR.
        uint32_t packed_color = Pixels(x, y);
        GRAPHICS::Color color = GRAPHICS::Color::Unpack(packed_color, ColorFormat);
        return color;
    }

    /// Fills in color of the pixel at the specified coordinates.
    /// @param[in]  x - The horizontal coordinate of the pixel.
    /// @param[in]  y - The vertical coorindate of the pixel.
    /// @param[in]  color - The color to write to the pixel, already in 32-bit packed
    ///     format according to the color format specified for the bitmap.
    void Bitmap::WritePixel(const unsigned int x, const unsigned int y, const uint32_t& color)
    {
        // MAKE SURE THE PIXEL COORDINATES ARE VALID.
        bool pixel_coordinates_valid = Pixels.IndicesInRange(x, y);
        if (!pixel_coordinates_valid)
        {
            // The pixel can't be written.
            return;
        }

        // FILL IN THE COLOR COMPONENTS OF THE PIXEL.
        Pixels(x, y) = color;
    }

    /// Fills in color of the pixel at the specified coordinates.
    /// @param[in]  x - The horizontal coordinate of the pixel.
    /// @param[in]  y - The vertical coorindate of the pixel.
    /// @param[in]  color - The color to write to the pixel.
    void Bitmap::WritePixel(const unsigned int x, const unsigned int y, const Color& color)
    {
        // MAKE SURE THE PIXEL COORDINATES ARE VALID.
        bool pixel_coordinates_valid = Pixels.IndicesInRange(x, y);
        if (!pixel_coordinates_valid)
        {
            // The pixel can't be written.
            return;
        }

        // FILL IN THE COLOR COMPONENTS OF THE PIXEL.
        uint32_t packed_color = color.Pack(ColorFormat);
        Pixels(x, y) = packed_color;
    }

    /// Fills all pixels in the bitmap with the specified color.
    /// @param[in]  color - The color to fill all pixels.
    void Bitmap::FillPixels(const Color& color)
    {
        // FILL IN ALL PIXELS.
        for (unsigned int y = 0; y < HeightInPixels; ++y)
        {
            for (unsigned int x = 0; x < WidthInPixels; ++x)
            {
                WritePixel(x, y, color);
            }
        }
    }
}
