#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/DisplayMode.h"

namespace GRAPHICS::DIRECT_X
{
    /// Gets the smallest matching display mode for the specified resolution, if one exists.
    /// @param[in]  width_in_pixels - The minimum width in pixels for the display mode.
    /// @param[in]  height_in_pixels - The minimum height in pixels for the display mode.
    /// @return The smallest matching display mode, if found; null otherwise.
    std::optional<DXGI_MODE_DESC> DisplayMode::GetSmallestMatching(
        const unsigned int width_in_pixels,
        const unsigned int height_in_pixels)
    {
        // CREATE THE FACTORY FOR GETTING DIRECT X GRAPHICS OBJECTS.
        IDXGIFactory* direct_x_graphics = nullptr;
        HRESULT create_direct_x_graphics_factory_result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&direct_x_graphics);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_direct_x_graphics_factory_result)
        {
            // INDICATE THAT NO MATCHING DISPLAY MODE COULD BE FOUND.
            return std::nullopt;
        }

        // SEARCH DISPLAY ADAPTERS FOR THE SMALLEST MATCHING DISPLAY MODE.
        std::optional<DXGI_MODE_DESC> smallest_matching_display_mode;
        IDXGIAdapter* display_adapter = nullptr;
        constexpr HRESULT FAILED_RESULT = -1;
        HRESULT get_display_adapter_result = FAILED_RESULT;
        for (UINT display_adapter_index = 0;
            (get_display_adapter_result = direct_x_graphics->EnumAdapters(display_adapter_index, &display_adapter)) != DXGI_ERROR_NOT_FOUND;
            ++display_adapter_index)
        {
            // MAKE SURE A DISPLAY ADAPTER WAS FOUND. 
            ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_display_adapter_result)
            {
                // CONTINUE SEARCHING FOR OTHER DISPLAY ADAPTERS.
                continue;
            }
            
            // PROVIDE DEBUG VISIBILITY INTO THE DISPLAY ADAPTER INDEX.
            std::string display_adapter_message = "\nDisplay adapter index = " + std::to_string(display_adapter_index);
            OutputDebugString(display_adapter_message.c_str());

            // SEE IF THIS ADAPTER HAS MATCHING OUTPUT CAPABILITIES.
            IDXGIOutput* display_adapter_output = nullptr;
            HRESULT get_display_adapter_output_result = FAILED_RESULT;
            for (UINT display_adapter_output_index = 0;
                (get_display_adapter_output_result = display_adapter->EnumOutputs(display_adapter_output_index, &display_adapter_output)) != DXGI_ERROR_NOT_FOUND;
                ++display_adapter_output_index)
            {
                // MAKE SURE A DISPLAY ADAPTER OUTPUT WAS FOUND.
                ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_display_adapter_output_result)
                {
                    // CONTINUE SEARCHING FOR OTHER DISPLAY ADAPTERS.
                    continue;
                }
                
                // PROVIDE DEBUG VISIBILITY INTO THE DISPLAY ADAPTER OUTPUT INDEX.
                std::string display_adapter_output_message = "\nDisplay adapter output index = " + std::to_string(display_adapter_output_index);
                OutputDebugString(display_adapter_output_message.c_str());

                // SEARCH FOR COMPATIBLE DISPLAY FORMATS.
                std::vector<DXGI_FORMAT> display_formats =
                {
                    //DXGI_FORMAT_UNKNOWN, // 0
                    //DXGI_FORMAT_R8G8B8A8_TYPELESS, // 27 (41 results)
                    DXGI_FORMAT_R8G8B8A8_UNORM, // 28 (41 results)
                    //DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // 29
                    //DXGI_FORMAT_R8G8B8A8_UINT, // 30
                    //DXGI_FORMAT_R8G8B8A8_SNORM, // 31
                    //DXGI_FORMAT_R8G8B8A8_SINT, // 32 (41 results)
    #if BGRA_FORMATS
                    DXGI_FORMAT_B8G8R8A8_UNORM, // 87
                    DXGI_FORMAT_B8G8R8X8_UNORM, // 88
                    DXGI_FORMAT_B8G8R8A8_TYPELESS, // 90 (41 results)
                    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, // 91
                    DXGI_FORMAT_B8G8R8X8_TYPELESS, // 92
                    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, // 93
    #endif
                };
                for (const DXGI_FORMAT display_format : display_formats)
                {
                    // GET THE NUMBER DISPLAY MODES FOR THE CURRENT FORMAT.
                    UINT display_mode_count = 0;
                    constexpr DXGI_MODE_DESC* GET_ALL_DISPLAY_MODES = nullptr;
                    HRESULT get_display_mode_count_result = display_adapter_output->GetDisplayModeList(
                        display_format,
                        DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING,
                        &display_mode_count,
                        GET_ALL_DISPLAY_MODES);
                    ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_display_mode_count_result)
                    {
                        // CONTINUE CHECKING OTHER DISPLAY FORMATS.
                        continue;
                    }

                    // PROVIDE DEBUG VISIBILITY INTO THE NUMBER OF DISPLAY MODES FOUND.
                    std::string display_mode_count_message = "\nDisplay mode count = " + std::to_string(display_mode_count);
                    OutputDebugString(display_mode_count_message.c_str());

                    // GET ALL DISPLAY MODES FOR THE CURRENT FORMAT.
                    std::unique_ptr<DXGI_MODE_DESC[]> display_mode_descriptions = std::make_unique<DXGI_MODE_DESC[]>(display_mode_count);
                    HRESULT get_display_modes_result = display_adapter_output->GetDisplayModeList(
                        display_format,
                        DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING,
                        &display_mode_count,
                        display_mode_descriptions.get());
                    ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_display_modes_result)
                    {
                        // CONTINUE CHECKING OTHER DISPLAY FORMATS.
                        continue;
                    }

                    // SEARCH THE DISPLAY MODES FOR THE SMALLEST MATCHING ONE.
                    for (std::size_t display_mode_index = 0; display_mode_index < display_mode_count; ++display_mode_index)
                    {
                        // PROVIDE DEBUG VISIBILITY INTO THE CURRENT DISPLAY MODE.
                        const DXGI_MODE_DESC& display_mode_description = display_mode_descriptions[display_mode_index];
                        std::string display_mode_message =
                            "\nDisplay mode: " + std::to_string(display_mode_index) +
                            "\n\tWidth = " + std::to_string(display_mode_description.Width) +
                            "\n\tHeight = " + std::to_string(display_mode_description.Height) +
                            "\n\tRefreshRate = " +
                            std::to_string(display_mode_description.RefreshRate.Numerator) +
                            "/" +
                            std::to_string(display_mode_description.RefreshRate.Denominator) +
                            "\n\tFormat = " + std::to_string(display_mode_description.Format) +
                            "\n\tScanlineOrdering = " + std::to_string(display_mode_description.ScanlineOrdering) +
                            "\n\tScaling = " + std::to_string(display_mode_description.Scaling);
                        OutputDebugString(display_mode_message.c_str());

                        // CHECK FOR A SMALLEST MATCHING DISPLAY MODE IF ONE HAS NOT YET BEEN FOUND.
                        if (!smallest_matching_display_mode)
                        {
                            // CHECK IF THE CURRENT DISPLAY MODE IS BIG ENOUGH.
                            bool current_display_mode_big_enough = (
                                display_mode_description.Width >= width_in_pixels &&
                                display_mode_description.Height >= height_in_pixels);
                            if (current_display_mode_big_enough)
                            {
                                // TRACK THE CURRENT DISPLAY MODE AS THE SMALLEST MATCHING ONE.
                                smallest_matching_display_mode = display_mode_description;
                            }
                        }
                    }
                }

                // PROVIDE DEBUG VISIBILITY INTO THE SMALLEST MATCHING DISPLAY MODE IF ONE WAS FOUND.
                if (smallest_matching_display_mode)
                {
                    std::string display_mode_message = std::string("\nMatching display mode: ") +
                        "\n\tWidth = " + std::to_string(smallest_matching_display_mode->Width) +
                        "\n\tHeight = " + std::to_string(smallest_matching_display_mode->Height) +
                        "\n\tRefreshRate = " +
                        std::to_string(smallest_matching_display_mode->RefreshRate.Numerator) +
                        "/" +
                        std::to_string(smallest_matching_display_mode->RefreshRate.Denominator) +
                        "\n\tFormat = " + std::to_string(smallest_matching_display_mode->Format) +
                        "\n\tScanlineOrdering = " + std::to_string(smallest_matching_display_mode->ScanlineOrdering) +
                        "\n\tScaling = " + std::to_string(smallest_matching_display_mode->Scaling);
                    OutputDebugString(display_mode_message.c_str());
                }

                // RELEASE THE DISPPLAY ADAPTER OUTPUT.
                display_adapter_output->Release();
            }

            // PROVIDE DEBUG INFROMATION INTO THE DISPLAY ADAPTER DESCRIPTION.
            DXGI_ADAPTER_DESC display_adapter_description;
            HRESULT get_display_adapter_description_result = display_adapter->GetDesc(&display_adapter_description);
            ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_display_adapter_description_result)
            {
                // https://en.cppreference.com/w/cpp/locale/wstring_convert
                OutputDebugStringW(L"\n");
                OutputDebugStringW(display_adapter_description.Description);
                std::string display_adapter_description_message =
                    "\n\tVendorId = " + std::to_string(display_adapter_description.VendorId) +
                    "\n\tDeviceId = " + std::to_string(display_adapter_description.DeviceId) +
                    "\n\tSubSysId = " + std::to_string(display_adapter_description.SubSysId) +
                    "\n\tDedicatedVideoMemory = " + std::to_string(display_adapter_description.DedicatedVideoMemory) +
                    "\n\tDedicatedSystemMemory = " + std::to_string(display_adapter_description.DedicatedSystemMemory) +
                    "\n\tSharedSystemMemory = " + std::to_string(display_adapter_description.SharedSystemMemory) +
                    "\n\tAdapterLuid high = " + std::to_string(display_adapter_description.AdapterLuid.HighPart) +
                    "\n\tAdapterLuid low = " + std::to_string(display_adapter_description.AdapterLuid.LowPart);
                OutputDebugString(display_adapter_description_message.c_str());
            }

            // RELEASE THE DISPLAY ADAPTER.
            display_adapter->Release();
        }

        // RELEASE THE DIRECT X GRAPHICS FACTORY.
        direct_x_graphics->Release();

        // RETURN THE SMALLEST MATCHING DISPLAY MODE, IF FOUND.
        return smallest_matching_display_mode;
    }
}
